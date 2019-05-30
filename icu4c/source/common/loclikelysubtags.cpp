// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// loclikelysubtags.h
// created: 2019may08 Markus W. Scherer

#ifndef __LOCLIKELYSUBTAGS_H__
#define __LOCLIKELYSUBTAGS_H__

#include "unicode/utypes.h"
#include "unicode/bytestrie.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"
#include "unicode/ures.h"
#include "charstr.h"
#include "uinvchar.h"

U_NAMESPACE_BEGIN

/** const char * keys & values. */
class CharStringMap final : public UMemory {
public:
    CharStringMap();
    CharStringMap(CharStringMap &&other);
    CharStringMap(const CharStringMap &other) = delete;
    ~CharStringMap();

    CharStringMap &operator=(CharStringMap &&other);
    CharStringMap &operator=(const CharStringMap &other) = delete;

    const char *get(const char *key);
    // TODO: ownership?
    void put(const char *key, const char *value, UErrorCode &errorCode);

private:
};

namespace {

constexpr char PSEUDO_ACCENTS_PREFIX = '\'';  // -XA, -PSACCENT
constexpr char PSEUDO_BIDI_PREFIX = '+';  // -XB, -PSBIDI
constexpr char PSEUDO_CRACKED_PREFIX = ',';  // -XC, -PSCRACK

}  // namespace

class XLikelySubtags final : public UMemory {
public:
    static constexpr int32_t SKIP_SCRIPT = 1;

    // VisibleForTesting -- TODO: ??
    static struct Data final {
        CharStringMap languageAliases;
        CharStringMap regionAliases;
        BytesTrie trie;
        const LSR *lsrs;

        Data(CharStringMap langAliases, CharStringMap rAliases, BytesTrie trie, const LSR lsrs[]) :
                languageAliases(langAliases),
                regionAliases(rAliases),
                trie(trie),
                lsrs(lsrs) {}
#if 0
        // VisibleForTesting
        public static Data load(UErrorCode &errorCode) {
            ICUResourceBundle langInfo = ICUResourceBundle.getBundleInstance(
                    ICUData.ICU_BASE_NAME, "langInfo",
                    ICUResourceBundle.ICU_DATA_CLASS_LOADER, ICUResourceBundle.OpenType.DIRECT);
            UResource.Value value = langInfo.getValueWithFallback("likely");
            UResource.Table likelyTable = value.getTable();

            CharStringMap languageAliases;
            if (likelyTable.findValue("languageAliases", value)) {
                String[] pairs = value.getStringArray();
                languageAliases = new HashMap<>(pairs.length / 2);
                for (int i = 0; i < pairs.length; i += 2) {
                    languageAliases.put(pairs[i], pairs[i + 1], errorCode);
                }
            }

            CharStringMap regionAliases;
            if (likelyTable.findValue("regionAliases", value)) {
                String[] pairs = value.getStringArray();
                regionAliases = new HashMap<>(pairs.length / 2);
                for (int i = 0; i < pairs.length; i += 2) {
                    regionAliases.put(pairs[i], pairs[i + 1], errorCode);
                }
            }

            ByteBuffer buffer = getValue(likelyTable, "trie", value).getBinary();
            byte[] trie = new byte[buffer.remaining()];
            buffer.get(trie);

            String[] lsrSubtags = getValue(likelyTable, "lsrs", value).getStringArray();
            LSR[] lsrs = new LSR[lsrSubtags.length / 3];
            for (int i = 0, j = 0; i < lsrSubtags.length; i += 3, ++j) {
                lsrs[j] = new LSR(lsrSubtags[i], lsrSubtags[i + 1], lsrSubtags[i + 2]);
            }

            return new Data(languageAliases, regionAliases, trie, lsrs);
        }
#endif
    private:
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
    }

    // VisibleForTesting
    // TODO: public static final XLikelySubtags INSTANCE = new XLikelySubtags(Data.load());
    static const XLikelySubtags &getSingleton();

    // VisibleForTesting
    LSR makeMaximizedLsrFrom(const Locale &locale) const {
        const char *name = locale.getName();
        if (uprv_isAtSign(name[0]) && name[1] == 'x' && name[2] == '=') {  // name.startsWith("@x=")
            // Private use language tag x-subtag-subtag...
            return new LSR(name, "", "");
            // TODO: think about lifetime of LSR.language
        }
        return makeMaximizedLsr(locale.getLanguage(), locale.getScript(), locale.getCountry(),
                                locale.getVariant());
    }

    LSR makeMaximizedLsrFrom(Locale locale) const {
        const char *tag = locale.toLanguageTag();
        if (tag.startsWith("x-")) {
            // Private use language tag x-subtag-subtag...
            return new LSR(tag, "", "");
        }
        return makeMaximizedLsr(locale.getLanguage(), locale.getScript(), locale.getCountry(),
                locale.getVariant());
    }
#if 0
    // TODO: declare here, define last
    LSR minimizeSubtags(const char *languageIn, const char *scriptIn, const char *regionIn,
                        ULocale.Minimize fieldToFavor) const {
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
        assert value >= 0;
        if (value == 0) {
            value = trieNext(iter, "", 0);
            assert value >= 0;
            if (value == 0) {
                value = trieNext(iter, "", 0);
            }
        }
        assert value > 0;
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
private:
    CharStringMap languageAliases;
    CharStringMap regionAliases;

    // The trie maps each lang+script+region (encoded in ASCII) to an index into lsrs.
    // There is also a trie value for each intermediate lang and lang+script.
    // '*' is used instead of "und", "Zzzz"/"" and "ZZ"/"".
    BytesTrie trie;
    long trieUndState;
    long trieUndZzzzState;
    int defaultLsrIndex;
    long[] trieFirstLetterStates = new long[26];
    LSR[] lsrs;

    XLikelySubtags(XLikelySubtags.Data data) {
        languageAliases = data.languageAliases;
        regionAliases = data.regionAliases;
        trie = new BytesTrie(data.trie, 0);
        lsrs = data.lsrs;

        // Cache the result of looking up language="und" encoded as "*", and "und-Zzzz" ("**").
        UStringTrieResult result = trie.next('*');
        assert result.hasNext();
        trieUndState = trie.getState64();
        result = trie.next('*');
        assert result.hasNext();
        trieUndZzzzState = trie.getState64();
        result = trie.next('*');
        assert result.hasValue();
        defaultLsrIndex = trie.getValue();
        trie.reset();

        for (char c = 'a'; c <= 'z'; ++c) {
            result = trie.next(c);
            if (result == USTRINGTRIE_NO_VALUE) {
                trieFirstLetterStates[c - 'a'] = trie.getState64();
            }
            trie.reset();
        }

        if (DEBUG_OUTPUT) {
            System.out.println("*** likely subtags");
            for (Map.Entry<String, LSR> mapping : getTable().entrySet()) {
                System.out.println(mapping);
            }
        }
    }

#if 0
    /**
     * Implementation of LocaleMatcher.canonicalize(ULocale).
     */
    public ULocale canonicalize(ULocale locale) {
        const char *lang = locale.getLanguage();
        const char *lang2 = languageAliases.get(lang);
        const char *region = locale.getCountry();
        const char *region2 = regionAliases.get(region);
        if (lang2 != null || region2 != null) {
            return new ULocale(
                lang2 == null ? lang : lang2,
                locale.getScript(),
                region2 == null ? region : region2);
        }
        return locale;
    }
#endif

    static const char *getCanonical(CharStringMap aliases, const char *alias) {
        const char *canonical = aliases.get(alias);
        return canonical == nullptr ? alias : canonical;
    }

    LSR makeMaximizedLsr(const char *language, const char *script, const char *region,
                         const char *variant) const {
        // Handle pseudolocales like en-XA, ar-XB, fr-PSCRACK.
        // They should match only themselves,
        // not other locales with what looks like the same language and script subtags.
        if (region.length() == 2 && region.charAt(0) == 'X') {
            switch (region.charAt(1)) {
            case 'A':
                return new LSR(PSEUDO_ACCENTS_PREFIX + language,
                        PSEUDO_ACCENTS_PREFIX + script, region);
            case 'B':
                return new LSR(PSEUDO_BIDI_PREFIX + language,
                        PSEUDO_BIDI_PREFIX + script, region);
            case 'C':
                return new LSR(PSEUDO_CRACKED_PREFIX + language,
                        PSEUDO_CRACKED_PREFIX + script, region);
            default:  // normal locale
                break;
            }
        }

        if (variant.startsWith("PS")) {
            switch (variant) {
            case "PSACCENT":
                return new LSR(PSEUDO_ACCENTS_PREFIX + language,
                        PSEUDO_ACCENTS_PREFIX + script, region.isEmpty() ? "XA" : region);
            case "PSBIDI":
                return new LSR(PSEUDO_BIDI_PREFIX + language,
                        PSEUDO_BIDI_PREFIX + script, region.isEmpty() ? "XB" : region);
            case "PSCRACK":
                return new LSR(PSEUDO_CRACKED_PREFIX + language,
                        PSEUDO_CRACKED_PREFIX + script, region.isEmpty() ? "XC" : region);
            default:  // normal locale
                break;
            }
        }

        language = getCanonical(languageAliases, language);
        // (We have no script mappings.)
        region = getCanonical(regionAliases, region);
        return INSTANCE.maximize(language, script, region);
    }

    /**
     * Raw access to addLikelySubtags. Input must be in canonical format, eg "en", not "eng" or "EN".
     */
    LSR maximize(const char *language, const char *script, const char *region) {
        if (language.equals("und")) {
            language = "";
        }
        if (script.equals("Zzzz")) {
            script = "";
        }
        if (region.equals("ZZ")) {
            region = "";
        }
        if (!script.isEmpty() && !region.isEmpty() && !language.isEmpty()) {
            return new LSR(language, script, region);  // already maximized
        }

        int retainOldMask = 0;
        BytesTrie iter = new BytesTrie(trie);
        long state;
        int value;
        // Small optimization: Array lookup for first language letter.
        int c0;
        if (language.length() >= 2 && 0 <= (c0 = language.charAt(0) - 'a') && c0 <= 25 &&
                (state = trieFirstLetterStates[c0]) != 0) {
            value = trieNext(iter.resetToState64(state), language, 1);
        } else {
            value = trieNext(iter, language, 0);
        }
        if (value >= 0) {
            if (!language.isEmpty()) {
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
            if (!script.isEmpty()) {
                retainOldMask |= 2;
            }
        } else {
            value = trieNext(iter, script, 0);
            if (value >= 0) {
                if (!script.isEmpty()) {
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
                    assert value >= 0;
                    state = iter.getState64();
                }
            }
        }

        if (value > 0) {
            // Final value from just language or language+script.
            if (!region.isEmpty()) {
                retainOldMask |= 1;
            }
        } else {
            value = trieNext(iter, region, 0);
            if (value >= 0) {
                if (!region.isEmpty()) {
                    retainOldMask |= 1;
                }
            } else {
                retainOldMask |= 1;
                if (state == 0) {
                    value = defaultLsrIndex;
                } else {
                    iter.resetToState64(state);
                    value = trieNext(iter, "", 0);
                    assert value > 0;
                }
            }
        }
        LSR result = lsrs[value];

        if (language.isEmpty()) {
            language = "und";
        }

        if (retainOldMask == 0) {
            return result;
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
        return new LSR(language, script, region);
    }

    static int trieNext(BytesTrie &iter, const char *s, int32_t i) {
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
};

U_NAMESPACE_END

#endif  // __LOCLIKELYSUBTAGS_H__
