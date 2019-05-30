// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// lsr.cpp
// created: 2019may08 Markus W. Scherer

#include "unicode/utypes.h"
#include "cstring.h"
#include "lsr.h"

U_NAMESPACE_BEGIN

UBool LSR::operator==(const LSR &other) {
    return uprv_strcmp(language, other.language) == 0 &&
        uprv_strcmp(script, other.script) == 0 &&
        regionIndex == other.regionIndex &&
        // Compare regions in case both are ill-formed (and their indexes are 0).
        uprv_strcmp(region, other.region) == 0;
}

U_NAMESPACE_END
