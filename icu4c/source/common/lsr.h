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
    /** Index for region, negative if ill-formed. @see indexForRegion */
    int32_t regionIndex;

    LSR(const char *lang, const char *scr, const char *r) {
        language = lang;
        script = scr;
        region = r;
        regionIndex = indexForRegion(region);
    }
    LSR(LSR &&other);
    LSR(const LSR &other) = delete;
    ~LSR();

    LSR &operator=(LSR &&other);
    LSR &operator=(const LSR &other) = delete;

    /**
     * Returns a positive index (>0) for a well-formed region code.
     * Do not rely on a particular region->index mapping; it may change.
     * Returns 0 for ill-formed strings.
     */
    static int32_t indexForRegion(const char *region) {
        int32_t c = region[0];
        int32_t a = c - '0';
        if (0 <= a && a <= 9) {  // digits: "419"
            int32_t b = region[1] - '0';
            if (b < 0 || 9 < b) { return 0; }
            c = region[2] - '0';
            if (c < 0 || 9 < c || region[3] != 0) { return 0; }
            return (10 * a + b) * 10 + c + 1;
        } else {  // letters: "DE"
            a = upperOrdinal(c);
            if (a < 0 || 25 < a) { return 0; }
            int32_t b = upperOrdinal(region[1]);
            if (b < 0 || 25 < b || region[2] != 0) { return 0; }
            return 26 * a + b + 1001;
        }
        return 0;
    }

    UBool operator==(const LSR &other);

    inline UBool operator!=(const LSR &other) {
        return !operator==(other);
    }

private:
    // Like uinvchar.h U_UPPER_ORDINAL(x) but with validation.
    // Returns 0..25 for A..Z else a value outside 0..25.
    static inline int32_t upperOrdinal(int32_t c) {
#if U_CHARSET_FAMILY==U_ASCII_FAMILY
        return c - 'A';
#elif U_CHARSET_FAMILY==U_EBCDIC_FAMILY
        if (c <= 'I') { return c - 'A'; }
        if (c < 'J') { return -1; }
        if (c <= 'R') { return c - 'J' + 9; }
        if (c < 'S') { return -1; }
        return c - 'S' + 18;
#else
#   error Unknown charset family!
#endif
    }
};

U_NAMESPACE_END

#endif  // __LSR_H__
