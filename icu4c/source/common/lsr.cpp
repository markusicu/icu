// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// lsr.cpp
// created: 2019may08 Markus W. Scherer

#include "unicode/utypes.h"
#include "cmemory.h"
#include "cstring.h"
#include "lsr.h"

U_NAMESPACE_BEGIN

LSR::LSR(char prefix, const char *lang, const char *scr, const char *r, UErrorCode &errorCode) :
        language(nullptr), script(nullptr), region(r), owned(nullptr),
        regionIndex(indexForRegion(region)) {
    if (U_SUCCESS(errorCode)) {
        // Number of bytes for language + script with 2 prefixes and NUL terminators.
        int32_t langSize = uprv_strlen(lang) + 1, scriptSize = uprv_strlen(scr) + 1;
        int32_t langScriptSize = 2 + langSize + scriptSize;
        char *p = owned = (char *)uprv_malloc(langScriptSize);
        if (owned != nullptr) {
            language = p;
            *p++ = prefix;
            uprv_memcpy(p, lang, langSize);
            script = p += langSize;
            *p++ = prefix;
            uprv_memcpy(p, scr, scriptSize);
        } else {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
        }
    }
}

LSR::LSR(LSR &&other) U_NOEXCEPT :
        language(other.language), script(other.script), region(other.region),
        owned(other.owned), regionIndex(other.regionIndex) {
    if (owned != nullptr) {
        other.language = other.script = "";
        other.owned = nullptr;
    }
}

LSR::~LSR() {
    uprv_free(owned);
}

LSR &LSR::operator=(LSR &&other) U_NOEXCEPT {
    language = other.language;
    script = other.script;
    region = other.region;
    regionIndex = other.regionIndex;
    delete owned;
    owned = other.owned;
    if (owned != nullptr) {
        other.language = other.script = "";
        other.owned = nullptr;
    }
    return *this;
}

UBool LSR::operator==(const LSR &other) const {
    return uprv_strcmp(language, other.language) == 0 &&
        uprv_strcmp(script, other.script) == 0 &&
        regionIndex == other.regionIndex &&
        // Compare regions if both are ill-formed (and their indexes are 0).
        (regionIndex > 0 || uprv_strcmp(region, other.region) == 0);
}

namespace {

// Like uinvchar.h U_UPPER_ORDINAL(x) but with validation.
// Returns 0..25 for A..Z else a value outside 0..25.
inline int32_t upperOrdinal(int32_t c) {
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

}  // namespace

int32_t LSR::indexForRegion(const char *region) {
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

U_NAMESPACE_END
