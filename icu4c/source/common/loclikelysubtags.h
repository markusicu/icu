// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// loclikelysubtags.h
// created: 2019may08 Markus W. Scherer

#ifndef __LOCLIKELYSUBTAGS_H__
#define __LOCLIKELYSUBTAGS_H__

#include <utility>
#include "unicode/utypes.h"
#include "unicode/bytestrie.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"
#include "unicode/ures.h"
#include "lsr.h"
#include "uhash.h"

U_NAMESPACE_BEGIN

struct XLikelySubtagsData;

/**
 * Map of const char * keys & values.
 * Stores pointers as is: Does not own/copy/adopt/release strings.
 */
class CharStringMap final : public UMemory {
public:
    /** Constructs an unusable non-map. */
    CharStringMap() : map(nullptr) {}
    CharStringMap(int32_t size, UErrorCode &errorCode) {
        map = uhash_openSize(uhash_hashChars, uhash_compareChars, uhash_compareChars,
                             size, &errorCode);
    }
    CharStringMap(CharStringMap &&other) U_NOEXCEPT : map(other.map) {
        other.map = nullptr;
    }
    CharStringMap(const CharStringMap &other) = delete;
    ~CharStringMap() {
        uhash_close(map);
    }

    CharStringMap &operator=(CharStringMap &&other) U_NOEXCEPT {
        map = other.map;
        other.map = nullptr;
        return *this;
    }
    CharStringMap &operator=(const CharStringMap &other) = delete;

    const char *get(const char *key) const { return static_cast<const char *>(uhash_get(map, key)); }
    void put(const char *key, const char *value, UErrorCode &errorCode) {
        uhash_put(map, const_cast<char *>(key), const_cast<char *>(value), &errorCode);
    }

private:
    UHashtable *map;
};

struct LocaleDistanceData {
    LocaleDistanceData(XLikelySubtagsData &data);
    ~LocaleDistanceData();

    const uint8_t *distanceTrieBytes = nullptr;
    const uint8_t *regionToPartitions = nullptr;
    const char **partitions = nullptr;
    const LSR *paradigms = nullptr;
    int32_t paradigmsLength = 0;
    const int32_t *distances = nullptr;
};

class XLikelySubtags final : public UMemory {
public:
    ~XLikelySubtags();

    static constexpr int32_t SKIP_SCRIPT = 1;

    // VisibleForTesting
    static const XLikelySubtags *getSingleton(UErrorCode &errorCode);

    // VisibleForTesting
    LSR makeMaximizedLsrFrom(const Locale &locale, UErrorCode &errorCode) const;

#if 0
    LSR minimizeSubtags(const char *languageIn, const char *scriptIn, const char *regionIn,
                        ULocale.Minimize fieldToFavor, UErrorCode &errorCode) const;
#endif

    // visible for LocaleDistance
    const LocaleDistanceData &getDistanceData() const { return distanceData; }

private:
    XLikelySubtags(XLikelySubtagsData &data);
    XLikelySubtags(const XLikelySubtags &other) = delete;
    XLikelySubtags &operator=(const XLikelySubtags &other) = delete;

    static void initLikelySubtags(UErrorCode &errorCode);

    LSR makeMaximizedLsr(const char *language, const char *script, const char *region,
                         const char *variant, UErrorCode &errorCode) const;

    /**
     * Raw access to addLikelySubtags. Input must be in canonical format, eg "en", not "eng" or "EN".
     */
    LSR maximize(const char *language, const char *script, const char *region) const;

    static int32_t trieNext(BytesTrie &iter, const char *s, int32_t i);

    UResourceBundle *langInfoBundle;
    CharString *strings;
    CharStringMap languageAliases;
    CharStringMap regionAliases;

    // The trie maps each lang+script+region (encoded in ASCII) to an index into lsrs.
    // There is also a trie value for each intermediate lang and lang+script.
    // '*' is used instead of "und", "Zzzz"/"" and "ZZ"/"".
    BytesTrie trie;
    uint64_t trieUndState;
    uint64_t trieUndZzzzState;
    int32_t defaultLsrIndex;
    uint64_t trieFirstLetterStates[26];
    const LSR *lsrs;
    int32_t lsrsLength;

    // distance/matcher data: see comment in XLikelySubtagsData::load()
    LocaleDistanceData distanceData;
};

U_NAMESPACE_END

#endif  // __LOCLIKELYSUBTAGS_H__
