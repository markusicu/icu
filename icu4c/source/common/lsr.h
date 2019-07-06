// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// lsr.h
// created: 2019may08 Markus W. Scherer

#ifndef __LSR_H__
#define __LSR_H__

#include "unicode/utypes.h"
#include "unicode/uobject.h"

U_NAMESPACE_BEGIN

struct LSR final : public UMemory {
    static constexpr int32_t REGION_INDEX_LIMIT = 1001 + 26 * 26;

    const char *language;
    const char *script;
    const char *region;
    char *owned;
    /** Index for region, negative if ill-formed. @see indexForRegion */
    int32_t regionIndex;

    LSR() : language("und"), script(""), region(""), owned(nullptr), regionIndex(indexForRegion(region)) {}

    /** Constructor which aliases all subtag pointers. */
    LSR(const char *lang, const char *scr, const char *r) :
            language(lang),  script(scr), region(r), owned(nullptr),
            regionIndex(indexForRegion(region)) {}
    /**
     * Constructor which prepends the prefix to the language and script,
     * copies those into owned memory, and aliases the region.
     */
    LSR(char prefix, const char *lang, const char *scr, const char *r, UErrorCode &errorCode);
    LSR(LSR &&other) U_NOEXCEPT;
    LSR(const LSR &other) = delete;
    ~LSR();

    LSR &operator=(LSR &&other) U_NOEXCEPT;
    LSR &operator=(const LSR &other) = delete;

    /**
     * Returns a positive index (>0) for a well-formed region code.
     * Do not rely on a particular region->index mapping; it may change.
     * Returns 0 for ill-formed strings.
     */
    static int32_t indexForRegion(const char *region);

    UBool operator==(const LSR &other) const;

    inline UBool operator!=(const LSR &other) const {
        return !operator==(other);
    }
};

U_NAMESPACE_END

#endif  // __LSR_H__
