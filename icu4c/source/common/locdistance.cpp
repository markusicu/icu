// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// locdistance.cpp
// created: 2019may08 Markus W. Scherer

#include "unicode/utypes.h"
#include "unicode/bytestrie.h"
#include "unicode/localematcher.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"
#include "unicode/ures.h"
#include "cstring.h"
#include "locdistance.h"
#include "uassert.h"
#include "uinvchar.h"

U_NAMESPACE_BEGIN

namespace {

/** Distance value bit flag, set by the builder. */
constexpr int32_t DISTANCE_SKIP_SCRIPT = 0x80;
/** Distance value bit flag, set by trieNext(). */
constexpr int32_t DISTANCE_IS_FINAL = 0x100;
constexpr int32_t DISTANCE_IS_FINAL_OR_SKIP_SCRIPT = DISTANCE_IS_FINAL | DISTANCE_SKIP_SCRIPT;

constexpr int32_t ABOVE_THRESHOLD = 100;

// Indexes into array of distances.
enum {
    IX_DEF_LANG_DISTANCE,
    IX_DEF_SCRIPT_DISTANCE,
    IX_DEF_REGION_DISTANCE,
    IX_MIN_REGION_DISTANCE,
    IX_LIMIT
};

}  // namespace
#if 0
// TODO: VisibleForTesting??
struct LocaleDistanceData final {
    const uint8_t *trie;
    const uint8_t *regionToPartitionsIndex;
    const char **partitionArrays;
    LSR *paradigmLSRs;
    int32_t paradigmLSRsLength;
    int32_t *distances;

    public LocaleDistanceData(byte[] trie,
            byte[] regionToPartitionsIndex, String[] partitionArrays,
            Set<LSR> paradigmLSRs, int32_t[] distances) {
        this.trie = trie;
        this.regionToPartitionsIndex = regionToPartitionsIndex;
        this.partitionArrays = partitionArrays;
        this.paradigmLSRs = paradigmLSRs;
        this.distances = distances;
    }

    private static UResource.Value getValue(UResource.Table table,
            String key, UResource.Value value) {
        if (!table.findValue(key, value)) {
            throw new MissingResourceException(
                    "langInfo.res missing data", "", "match/" + key);
        }
        return value;
    }

    // VisibleForTesting
    static LocaleDistanceData load(UErrorCode &errorCode) {
        ICUResourceBundle langInfo = ICUResourceBundle.getBundleInstance(
                ICUData.ICU_BASE_NAME, "langInfo",
                ICUResourceBundle.ICU_DATA_CLASS_LOADER, ICUResourceBundle.OpenType.DIRECT);
        UResource.Value value = langInfo.getValueWithFallback("match");
        UResource.Table matchTable = value.getTable();

        ByteBuffer buffer = getValue(matchTable, "trie", value).getBinary();
        byte[] trie = new byte[buffer.remaining()];
        buffer.get(trie);

        buffer = getValue(matchTable, "regionToPartitions", value).getBinary();
        byte[] regionToPartitions = new byte[buffer.remaining()];
        buffer.get(regionToPartitions);
        if (regionToPartitions.length < LSR.REGION_INDEX_LIMIT) {
            throw new MissingResourceException(
                    "langInfo.res binary data too short", "", "match/regionToPartitions");
        }

        String[] partitions = getValue(matchTable, "partitions", value).getStringArray();

        Set<LSR> paradigmLSRs;
        if (matchTable.findValue("paradigms", value)) {
            String[] paradigms = value.getStringArray();
            paradigmLSRs = new HashSet<>(paradigms.length / 3);
            for (int32_t i = 0; i < paradigms.length; i += 3) {
                paradigmLSRs.add(new LSR(paradigms[i], paradigms[i + 1], paradigms[i + 2]));
            }
        } else {
            paradigmLSRs = Collections.emptySet();
        }

        int32_t[] distances = getValue(matchTable, "distances", value).getIntVector();
        if (distances.length < IX_LIMIT) {
            throw new MissingResourceException(
                    "langInfo.res intvector too short", "", "match/distances");
        }

        return LocaleDistanceData(trie, regionToPartitions, partitions, paradigmLSRs, distances);
    }
}
#endif
// TODO: VisibleForTesting
// TODO: public static final LocaleDistance INSTANCE = new LocaleDistance(Data.load());
#if 0
private LocaleDistance(LocaleDistanceData &data) {
    this.trie = new BytesTrie(data.trie, 0);
    this.regionToPartitionsIndex = data.regionToPartitionsIndex;
    this.partitionArrays = data.partitionArrays;
    this.paradigmLSRs = data.paradigmLSRs;
    defaultLanguageDistance = data.distances[IX_DEF_LANG_DISTANCE];
    defaultScriptDistance = data.distances[IX_DEF_SCRIPT_DISTANCE];
    defaultRegionDistance = data.distances[IX_DEF_REGION_DISTANCE];
    this.minRegionDistance = data.distances[IX_MIN_REGION_DISTANCE];

    LSR en = new LSR("en", "Latn", "US");
    LSR enGB = new LSR("en", "Latn", "GB");
    defaultDemotionPerDesiredLocale = getBestIndexAndDistance(en, &enGB, 1,
            50, ULOCMATCH_FAVOR_LANGUAGE) & 0xff;
}
#endif
#if 0
// TODO: VisibleForTesting
int32_t testOnlyDistance(const Locale &desired, const Locale &supported,
                         int32_t threshold, ULocMatchFavorSubtag favorSubtag,
                         UErrorCode &errorCode) const {
    const XLikelySubtags &likely = XLikelySubtags::getSingleton(errorCode);
    if (U_FAILURE(errorCode)) { return ABOVE_THRESHOLD; }
    LSR supportedLSR = likely.makeMaximizedLsrFrom(supported);
    LSR desiredLSR = likely.makeMaximizedLsrFrom(desired);
    return getBestIndexAndDistance(desiredLSR, &supportedLSR, 1,
            threshold, favorSubtag) & 0xff;
}
#endif

int32_t LocaleDistance::getBestIndexAndDistance(
        const LSR &desired,
        const LSR *supportedLsrs, int32_t supportedLsrsLength,
        int32_t threshold, ULocMatchFavorSubtag favorSubtag) const {
    BytesTrie iter(trie);
    // Look up the desired language only once for all supported LSRs.
    // Its "distance" is either a match point value of 0, or a non-match negative value.
    // Note: The data builder verifies that there are no <*, supported> or <desired, *> rules.
    int32_t desLangDistance = trieNext(iter, desired.language, false);
    uint64_t desLangState = desLangDistance >= 0 && supportedLsrsLength > 1 ? iter.getState64() : 0;
    // Index of the supported LSR with the lowest distance.
    int32_t bestIndex = -1;
    for (int32_t slIndex = 0; slIndex < supportedLsrsLength; ++slIndex) {
        const LSR &supported = supportedLsrs[slIndex];
        bool star = false;
        int32_t distance = desLangDistance;
        if (distance >= 0) {
            U_ASSERT((distance & DISTANCE_IS_FINAL) == 0);
            if (slIndex != 0) {
                iter.resetToState64(desLangState);
            }
            distance = trieNext(iter, supported.language, true);
        }
        // Note: The data builder verifies that there are no rules with "any" (*) language and
        // real (non *) script or region subtags.
        // This means that if the lookup for either language fails we can use
        // the default distances without further lookups.
        int32_t flags;
        if (distance >= 0) {
            flags = distance & DISTANCE_IS_FINAL_OR_SKIP_SCRIPT;
            distance &= ~DISTANCE_IS_FINAL_OR_SKIP_SCRIPT;
        } else {  // <*, *>
            if (uprv_strcmp(desired.language, supported.language) == 0) {
                distance = 0;
            } else {
                distance = defaultLanguageDistance;
            }
            flags = 0;
            star = true;
        }
        U_ASSERT(0 <= distance && distance <= 100);
        if (favorSubtag == ULOCMATCH_FAVOR_SCRIPT) {
            distance >>= 2;
        }
        if (distance >= threshold) {
            continue;
        }

        int32_t scriptDistance;
        if (star || flags != 0) {
            if (uprv_strcmp(desired.script, supported.script) == 0) {
                scriptDistance = 0;
            } else {
                scriptDistance = defaultScriptDistance;
            }
        } else {
            scriptDistance = getDesSuppScriptDistance(iter, iter.getState64(),
                    desired.script, supported.script);
            flags = scriptDistance & DISTANCE_IS_FINAL;
            scriptDistance &= ~DISTANCE_IS_FINAL;
        }
        distance += scriptDistance;
        if (distance >= threshold) {
            continue;
        }

        if (uprv_strcmp(desired.region, supported.region) == 0) {
            // regionDistance = 0
        } else if (star || (flags & DISTANCE_IS_FINAL) != 0) {
            distance += defaultRegionDistance;
        } else {
            int32_t remainingThreshold = threshold - distance;
            if (minRegionDistance >= remainingThreshold) {
                continue;
            }

            // From here on we know the regions are not equal.
            // Map each region to zero or more partitions. (zero = one non-matching string)
            // (Each array of single-character partition strings is encoded as one string.)
            // If either side has more than one, then we find the maximum distance.
            // This could be optimized by adding some more structure, but probably not worth it.
            distance += getRegionPartitionsDistance(
                    iter, iter.getState64(),
                    partitionsForRegion(desired),
                    partitionsForRegion(supported),
                    remainingThreshold);
        }
        if (distance < threshold) {
            if (distance == 0) {
                return slIndex << 8;
            }
            bestIndex = slIndex;
            threshold = distance;
        }
    }
    return bestIndex >= 0 ? (bestIndex << 8) | threshold : 0xffffff00 | ABOVE_THRESHOLD;
}

int32_t LocaleDistance::getDesSuppScriptDistance(
        BytesTrie &iter, uint64_t startState, const char *desired, const char *supported) {
    // Note: The data builder verifies that there are no <*, supported> or <desired, *> rules.
    int32_t distance = trieNext(iter, desired, false);
    if (distance >= 0) {
        distance = trieNext(iter, supported, true);
    }
    if (distance < 0) {
        UStringTrieResult result = iter.resetToState64(startState).next('*');  // <*, *>
        U_ASSERT(USTRINGTRIE_HAS_VALUE(result));
        if (uprv_strcmp(desired, supported) == 0) {
            distance = 0;  // same script
        } else {
            distance = iter.getValue();
            U_ASSERT(distance >= 0);
        }
        if (result == USTRINGTRIE_FINAL_VALUE) {
            distance |= DISTANCE_IS_FINAL;
        }
    }
    return distance;
}

int32_t LocaleDistance::getRegionPartitionsDistance(
        BytesTrie &iter, uint64_t startState,
        const char *desiredPartitions, const char *supportedPartitions, int32_t threshold) {
    char desired = *desiredPartitions++;
    char supported = *supportedPartitions++;
    U_ASSERT(desired != 0 && supported != 0);
    bool suppLengthGt1 = *supportedPartitions != 0;
    // if (desLength == 1 && suppLength == 1)
    if (*desiredPartitions == 0 && !suppLengthGt1) {
        UStringTrieResult result = iter.next(uprv_invCharToAscii(desired) | 0x80);
        if (USTRINGTRIE_HAS_NEXT(result)) {
            result = iter.next(uprv_invCharToAscii(supported) | 0x80);
            if (USTRINGTRIE_HAS_VALUE(result)) {
                return iter.getValue();
            }
        }
        return getFallbackRegionDistance(iter, startState);
    }

    int32_t regionDistance = 0;
    // Fall back to * only once, not for each pair of partition strings.
    bool star = false;
    for (;;) {
        // Look up each desired-partition string only once,
        // not for each (desired, supported) pair.
        UStringTrieResult result = iter.next(uprv_invCharToAscii(desired) | 0x80);
        if (USTRINGTRIE_HAS_NEXT(result)) {
            uint64_t desState = suppLengthGt1 ? iter.getState64() : 0;
            for (;;) {
                result = iter.next(uprv_invCharToAscii(supported) | 0x80);
                int32_t d;
                if (USTRINGTRIE_HAS_VALUE(result)) {
                    d = iter.getValue();
                } else if (star) {
                    d = 0;
                } else {
                    d = getFallbackRegionDistance(iter, startState);
                    star = true;
                }
                if (d >= threshold) {
                    return d;
                } else if (regionDistance < d) {
                    regionDistance = d;
                }
                if ((supported = *supportedPartitions++) != 0) {
                    iter.resetToState64(desState);
                } else {
                    break;
                }
            }
        } else if (!star) {
            int32_t d = getFallbackRegionDistance(iter, startState);
            if (d >= threshold) {
                return d;
            } else if (regionDistance < d) {
                regionDistance = d;
            }
            star = true;
        }
        if ((desired = *desiredPartitions++) != 0) {
            iter.resetToState64(startState);
        } else {
            break;
        }
    }
    return regionDistance;
}

int32_t LocaleDistance::getFallbackRegionDistance(BytesTrie &iter, uint64_t startState) {
    UStringTrieResult result = iter.resetToState64(startState).next('*');  // <*, *>
    U_ASSERT(USTRINGTRIE_HAS_VALUE(result));
    int32_t distance = iter.getValue();
    U_ASSERT(distance >= 0);
    return distance;
}

int32_t LocaleDistance::trieNext(BytesTrie &iter, const char *s, bool wantValue) {
    uint8_t c;
    if ((c = *s) == 0) {
        return -1;  // no empty subtags in the distance data
    }
    for (;;) {
        c = uprv_invCharToAscii(c);
        // EBCDIC: If *s is not an invariant character,
        // then c is now 0 and will simply not match anything, which is harmless.
        uint8_t next = *++s;
        if (next != 0) {
            if (!USTRINGTRIE_HAS_NEXT(iter.next(c))) {
                return -1;
            }
        } else {
            // last character of this subtag
            UStringTrieResult result = iter.next(c | 0x80);
            if (wantValue) {
                if (USTRINGTRIE_HAS_VALUE(result)) {
                    int32_t value = iter.getValue();
                    if (result == USTRINGTRIE_FINAL_VALUE) {
                        value |= DISTANCE_IS_FINAL;
                    }
                    return value;
                }
            } else {
                if (USTRINGTRIE_HAS_NEXT(result)) {
                    return 0;
                }
            }
            return -1;
        }
        c = next;
    }
}

#if 0
UBool LocaleDistance::isParadigmLSR(LSR lsr) const {
    return paradigmLSRs.contains(lsr);
}
#endif

U_NAMESPACE_END
