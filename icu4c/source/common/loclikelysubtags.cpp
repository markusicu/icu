// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// loclikelysubtags.cpp
// created: 2019may08 Markus W. Scherer

#include <utility>
#include "unicode/utypes.h"
#include "unicode/bytestrie.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"
#include "unicode/ures.h"
#include "charstr.h"
#include "cstring.h"
#include "loclikelysubtags.h"
#include "lsr.h"
#include "uassert.h"
#include "uinvchar.h"
#include "uresdata.h"
#include "uresimp.h"

U_NAMESPACE_BEGIN

namespace {

// Like uinvchar.h U_UPPER_ORDINAL(x) but for lowercase and with validation.
// Returns 0..25 for a..z else a value outside 0..25.
inline int32_t lowerOrdinal(int32_t c) {
#if U_CHARSET_FAMILY==U_ASCII_FAMILY
    return c - 'a';
#elif U_CHARSET_FAMILY==U_EBCDIC_FAMILY
    if (c <= 'i') { return c - 'a'; }
    if (c < 'j') { return -1; }
    if (c <= 'r') { return c - 'j' + 9; }
    if (c < 's') { return -1; }
    return c - 's' + 18;
#else
#   error Unknown charset family!
#endif
}

constexpr char PSEUDO_ACCENTS_PREFIX = '\'';  // -XA, -PSACCENT
constexpr char PSEUDO_BIDI_PREFIX = '+';  // -XB, -PSBIDI
constexpr char PSEUDO_CRACKED_PREFIX = ',';  // -XC, -PSCRACK

}  // namespace

// VisibleForTesting -- TODO: ??
struct XLikelySubtagsData {
    UResourceBundle *likelyBundle;
    CharStringMap languageAliases;
    CharStringMap regionAliases;
    BytesTrie trie;
    const LSR *lsrs;
    int32_t lsrsLength;

    XLikelySubtagsData(LocalUResourceBundlePointer likely,
                       CharStringMap langAliases, CharStringMap rAliases,
                       BytesTrie trie, const LSR lsrs[], int32_t lsrsLength) :
            likelyBundle(likely.orphan()),
            languageAliases(std::move(langAliases)),
            regionAliases(std::move(rAliases)),
            trie(trie),
            lsrs(lsrs), lsrsLength(lsrsLength) {}

    ~XLikelySubtagsData() {
        ures_close(likelyBundle);
    }

    // VisibleForTesting -- TODO: ??
    static XLikelySubtagsData *load(UErrorCode &errorCode) {
        LocalUResourceBundlePointer langInfo(ures_openDirect(nullptr, "langInfo", &errorCode));
        if (U_FAILURE(errorCode)) { return nullptr; }
        ResourceDataValue value;
        ures_getValueWithFallback(langInfo.getAlias(), "likely", value, errorCode);
        ResourceTable likelyTable = value.getTable(errorCode);
        if (U_FAILURE(errorCode)) { return nullptr; }

        // TODO: CharStringMap languageAliases;  // new HashMap<>(pairs.length / 2);
        if (!readAliases(likelyTable, "languageAliases", value, /*languageAliases,*/ errorCode)) {
            return nullptr;
        }

        // TODO: CharStringMap regionAliases;  // new HashMap<>(pairs.length / 2);
        if (!readAliases(likelyTable, "regionAliases", value, /*regionAliases,*/ errorCode)) {
            return nullptr;
        }

#if 0
        ByteBuffer buffer = getValue(likelyTable, "trie", value).getBinary();
        byte[] trie = new byte[buffer.remaining()];
        buffer.get(trie);

        String[] lsrSubtags = getValue(likelyTable, "lsrs", value).getStringArray();
        LSR[] lsrs = new LSR[lsrSubtags.length / 3];
        for (int i = 0, j = 0; i < lsrSubtags.length; i += 3, ++j) {
            lsrs[j] = new LSR(lsrSubtags[i], lsrSubtags[i + 1], lsrSubtags[i + 2]);
        }

        XLikelySubtagsData *data = new XLikelySubtagsData(languageAliases, regionAliases, trie, lsrs, lsrsLength);
        if (data == nullptr) {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
        }
        return data;
#endif
        return nullptr;
    }

private:
    static bool readAliases(const ResourceTable &likelyTable, const char *key, ResourceValue &value,
                            /* TODO: CharStringMap &map,*/ UErrorCode &errorCode) {
        if (likelyTable.findValue(key, value)) {
            ResourceArray pairs = value.getArray(errorCode);
            if (U_FAILURE(errorCode)) { return false; }
            int32_t pairsLength = pairs.getSize();
            UnicodeString from, to;
            CharString invFrom, invTo;  // invariant characters
            for (int i = 0; i < pairsLength; i += 2) {
                pairs.getValue(i, value);  // returns TRUE because i < pairsLength
                from = value.getUnicodeString(errorCode);
                invFrom.clear().appendInvariantChars(from, errorCode);
                if (U_FAILURE(errorCode)) { return false; }
                if (!pairs.getValue(i + 1, value)) { break; }
                to = value.getUnicodeString(errorCode);
                invTo.clear().appendInvariantChars(to, errorCode);
                if (U_FAILURE(errorCode)) { return false; }
                // TODO: map.put(invFrom.data(), invTo.data(), errorCode);
            }
        }
        return true;
    }

#if 0
    static UResource.Value getValue(UResource.Table table,
            const char *key, UResource.Value value) {
        if (!table.findValue(key, value)) {
            throw new MissingResourceException(
                    "langInfo.res missing data", "", "likely/" + key);
        }
        return value;
    }
#endif
};

// TODO: public static final XLikelySubtags INSTANCE = new XLikelySubtags(Data.load());
// TODO: const XLikelySubtags &XLikelySubtags::getSingleton(UErrorCode &errorCode);

// VisibleForTesting
LSR XLikelySubtags::makeMaximizedLsrFrom(const Locale &locale, UErrorCode &errorCode) const {
    const char *name = locale.getName();
    if (uprv_isAtSign(name[0]) && name[1] == 'x' && name[2] == '=') {  // name.startsWith("@x=")
        // Private use language tag x-subtag-subtag...
        return LSR(name, "", "");
        // TODO: think about lifetime of LSR.language
    }
    return makeMaximizedLsr(locale.getLanguage(), locale.getScript(), locale.getCountry(),
                            locale.getVariant(), errorCode);
}

XLikelySubtags::XLikelySubtags(XLikelySubtagsData &data) :
        languageAliases(std::move(data.languageAliases)),
        regionAliases(std::move(data.regionAliases)),
        trie(data.trie),
        lsrs(data.lsrs),
        lsrsLength(data.lsrsLength) {
    // Cache the result of looking up language="und" encoded as "*", and "und-Zzzz" ("**").
    UStringTrieResult result = trie.next('*');
    U_ASSERT(USTRINGTRIE_HAS_NEXT(result));
    trieUndState = trie.getState64();
    result = trie.next('*');
    U_ASSERT(USTRINGTRIE_HAS_NEXT(result));
    trieUndZzzzState = trie.getState64();
    result = trie.next('*');
    U_ASSERT(USTRINGTRIE_HAS_VALUE(result));
    defaultLsrIndex = trie.getValue();
    trie.reset();

    // In EBCDIC, a-z are not contiguous.
    static const char *a_z = "abcdefghijklmnopqrstuvwxyz";
    for (int32_t i = 0; i < 26; ++i) {
        char c = a_z[i];
        result = trie.next(c);
        if (result == USTRINGTRIE_NO_VALUE) {
            trieFirstLetterStates[i] = trie.getState64();
        }
        trie.reset();
    }
}

namespace {

const char *getCanonical(const CharStringMap &aliases, const char *alias) {
    const char *canonical = aliases.get(alias);
    return canonical == nullptr ? alias : canonical;
}

}  // namespace

LSR XLikelySubtags::makeMaximizedLsr(const char *language, const char *script, const char *region,
                                     const char *variant, UErrorCode &errorCode) const {
    // Handle pseudolocales like en-XA, ar-XB, fr-PSCRACK.
    // They should match only themselves,
    // not other locales with what looks like the same language and script subtags.
    char c1;
    if (region[0] == 'X' && (c1 = region[1]) != 0 && region[2] == 0) {
        switch (c1) {
        case 'A':
            return LSR(PSEUDO_ACCENTS_PREFIX, language, script, region, errorCode);
        case 'B':
            return LSR(PSEUDO_BIDI_PREFIX, language, script, region, errorCode);
        case 'C':
            return LSR(PSEUDO_CRACKED_PREFIX, language, script, region, errorCode);
        default:  // normal locale
            break;
        }
    }

    if (variant[0] == 'P' && variant[1] == 'S') {
        if (uprv_strcmp(variant, "PSACCENT") == 0) {
            return LSR(PSEUDO_ACCENTS_PREFIX, language, script,
                       *region == 0 ? "XA" : region, errorCode);
        } else if (uprv_strcmp(variant, "PSBIDI") == 0) {
            return LSR(PSEUDO_BIDI_PREFIX, language, script,
                       *region == 0 ? "XB" : region, errorCode);
        } else if (uprv_strcmp(variant, "PSCRACK") == 0) {
            return LSR(PSEUDO_CRACKED_PREFIX, language, script,
                       *region == 0 ? "XC" : region, errorCode);
        }
        // else normal locale
    }

    language = getCanonical(languageAliases, language);
    // (We have no script mappings.)
    region = getCanonical(regionAliases, region);
    return maximize(language, script, region);
}

LSR XLikelySubtags::maximize(const char *language, const char *script, const char *region) const {
    if (uprv_strcmp(language, "und") == 0) {
        language = "";
    }
    if (uprv_strcmp(script, "Zzzz") == 0) {
        script = "";
    }
    if (uprv_strcmp(region, "ZZ") == 0) {
        region = "";
    }
    if (*script != 0 && *region != 0 && *language != 0) {
        return LSR(language, script, region);  // already maximized
    }

    uint32_t retainOldMask = 0;
    BytesTrie iter(trie);
    uint64_t state;
    int32_t value;
    // Small optimization: Array lookup for first language letter.
    int32_t c0;
    if (0 <= (c0 = lowerOrdinal(language[0])) && c0 <= 25 &&
            language[1] != 0 &&  // language.length() >= 2
            (state = trieFirstLetterStates[c0]) != 0) {
        value = trieNext(iter.resetToState64(state), language, 1);
    } else {
        value = trieNext(iter, language, 0);
    }
    if (value >= 0) {
        if (*language != 0) {
            retainOldMask |= 4;
        }
        state = iter.getState64();
    } else {
        retainOldMask |= 4;
        iter.resetToState64(trieUndState);  // "und" ("*")
        state = 0;
    }

    if (value > 0) {
        // Intermediate or final value from just language.
        if (value == SKIP_SCRIPT) {
            value = 0;
        }
        if (*script != 0) {
            retainOldMask |= 2;
        }
    } else {
        value = trieNext(iter, script, 0);
        if (value >= 0) {
            if (*script != 0) {
                retainOldMask |= 2;
            }
            state = iter.getState64();
        } else {
            retainOldMask |= 2;
            if (state == 0) {
                iter.resetToState64(trieUndZzzzState);  // "und-Zzzz" ("**")
            } else {
                iter.resetToState64(state);
                value = trieNext(iter, "", 0);
                U_ASSERT(value >= 0);
                state = iter.getState64();
            }
        }
    }

    if (value > 0) {
        // Final value from just language or language+script.
        if (*region != 0) {
            retainOldMask |= 1;
        }
    } else {
        value = trieNext(iter, region, 0);
        if (value >= 0) {
            if (*region != 0) {
                retainOldMask |= 1;
            }
        } else {
            retainOldMask |= 1;
            if (state == 0) {
                value = defaultLsrIndex;
            } else {
                iter.resetToState64(state);
                value = trieNext(iter, "", 0);
                U_ASSERT(value > 0);
            }
        }
    }
    U_ASSERT(value < lsrsLength);
    const LSR &result = lsrs[value];

    if (*language == 0) {
        language = "und";
    }

    if (retainOldMask == 0) {
        // Quickly return a copy of the lookup-result LSR
        // without new allocation of the subtags.
        return LSR(result.language, result.script, result.region);
    }
    if ((retainOldMask & 4) == 0) {
        language = result.language;
    }
    if ((retainOldMask & 2) == 0) {
        script = result.script;
    }
    if ((retainOldMask & 1) == 0) {
        region = result.region;
    }
    return LSR(language, script, region);
}

int32_t XLikelySubtags::trieNext(BytesTrie &iter, const char *s, int32_t i) {
    UStringTrieResult result;
    uint8_t c;
    if ((c = s[i]) == 0) {
        result = iter.next(u'*');
    } else {
        for (;;) {
            c = uprv_invCharToAscii(c);
            // EBCDIC: If s[i] is not an invariant character,
            // then c is now 0 and will simply not match anything, which is harmless.
            uint8_t next = s[++i];
            if (next != 0) {
                if (!USTRINGTRIE_HAS_NEXT(iter.next(c))) {
                    return -1;
                }
            } else {
                // last character of this subtag
                result = iter.next(c | 0x80);
                break;
            }
            c = next;
        }
    }
    switch (result) {
    case USTRINGTRIE_NO_MATCH: return -1;
    case USTRINGTRIE_NO_VALUE: return 0;
    case USTRINGTRIE_INTERMEDIATE_VALUE:
        U_ASSERT(iter.getValue() == SKIP_SCRIPT);
        return SKIP_SCRIPT;
    case USTRINGTRIE_FINAL_VALUE: return iter.getValue();
    default: return -1;
    }
}

#if 0
LSR XLikelySubtags::minimizeSubtags(const char *languageIn, const char *scriptIn,
                                    const char *regionIn, ULocale.Minimize fieldToFavor,
                                    UErrorCode &errorCode) const {
    LSR result = maximize(languageIn, scriptIn, regionIn);

    // We could try just a series of checks, like:
    // LSR result2 = addLikelySubtags(languageIn, "", "");
    // if result.equals(result2) return result2;
    // However, we can optimize 2 of the cases:
    //   (languageIn, "", "")
    //   (languageIn, "", regionIn)

    // value00 = lookup(result.language, "", "")
    BytesTrie iter = new BytesTrie(trie);
    int value = trieNext(iter, result.language, 0);
    U_ASSERT(value >= 0);
    if (value == 0) {
        value = trieNext(iter, "", 0);
        U_ASSERT(value >= 0);
        if (value == 0) {
            value = trieNext(iter, "", 0);
        }
    }
    U_ASSERT(value > 0);
    LSR value00 = lsrs[value];
    boolean favorRegionOk = false;
    if (result.script.equals(value00.script)) { //script is default
        if (result.region.equals(value00.region)) {
            return new LSR(result.language, "", "");
        } else if (fieldToFavor == ULocale.Minimize.FAVOR_REGION) {
            return new LSR(result.language, "", result.region);
        } else {
            favorRegionOk = true;
        }
    }

    // The last case is not as easy to optimize.
    // Maybe do later, but for now use the straightforward code.
    LSR result2 = maximize(languageIn, scriptIn, "");
    if (result2.equals(result)) {
        return new LSR(result.language, result.script, "");
    } else if (favorRegionOk) {
        return new LSR(result.language, "", result.region);
    }
    return result;
}
#endif

U_NAMESPACE_END
