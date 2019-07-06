// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// localematcher.cpp
// created: 2019may08 Markus W. Scherer

#ifndef __LOCMATCHER_H__
#define __LOCMATCHER_H__

#include <stdio.h>  // TODO

#include "unicode/utypes.h"
#include "unicode/localematcher.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"
#include "cstring.h"
#include "loclikelysubtags.h"
#include "locdistance.h"
#include "lsr.h"
#include "uassert.h"
#include "uhash.h"
#include "ustr_imp.h"
#include "uvector.h"

#define UND_LSR LSR("und", "", "")

/**
 * Indicator for the lifetime of desired-locale objects passed into the LocaleMatcher.
 *
 * @draft ICU 65
 */
enum ULocMatchLifetime {
    /**
     * Locale objects are temporary.
     * The matcher will make a copy of a locale that will be used beyond one function call.
     *
     * @draft ICU 65
     */
    ULOCMATCH_TEMPORARY_LOCALES,
    /**
     * Locale objects are stored at least as long as the matcher is used.
     * The matcher will keep only a pointer to a locale that will be used beyond one function call,
     * avoiding a copy.
     *
     * @draft ICU 65
     */
    ULOCMATCH_STORED_LOCALES  // TODO: permanent? cached? clone?
};
#ifndef U_IN_DOXYGEN
typedef enum ULocMatchLifetime ULocMatchLifetime;
#endif

U_NAMESPACE_BEGIN

LocaleMatcher::Result::Result(LocaleMatcher::Result &&src) U_NOEXCEPT :
        desiredLocale(src.desiredLocale),
        supportedLocale(src.supportedLocale),
        desiredIndex(src.desiredIndex),
        supportedIndex(src.supportedIndex),
        desiredIsOwned(src.desiredIsOwned) {
    if (desiredIsOwned) {
        src.desiredLocale = nullptr;
        src.desiredIndex = -1;
        src.desiredIsOwned = FALSE;
    }
}

LocaleMatcher::Result::~Result() {
    if (desiredIsOwned) {
        delete desiredLocale;
    }
}

LocaleMatcher::Result &LocaleMatcher::Result::operator=(LocaleMatcher::Result &&src) U_NOEXCEPT {
    desiredLocale = src.desiredLocale;
    supportedLocale = src.supportedLocale;
    desiredIndex = src.desiredIndex;
    supportedIndex = src.supportedIndex;
    desiredIsOwned = src.desiredIsOwned;

    if (desiredIsOwned) {
        src.desiredLocale = nullptr;
        src.desiredIndex = -1;
        src.desiredIsOwned = FALSE;
    }
    return *this;
}

#if 0
Locale LocaleMatcher::Result::makeResolvedLocale() {
    ULocale bestDesired = getDesiredULocale();
    ULocale serviceLocale = supportedULocale;
    if (!serviceLocale.equals(bestDesired) && bestDesired != null) {
        ULocale.Builder b = new ULocale.Builder().setLocale(serviceLocale);

        // Copy the region from bestDesired, if there is one.
        String region = bestDesired.getCountry();
        if (!region.isEmpty()) {
            b.setRegion(region);
        }

        // Copy the variants from bestDesired, if there are any.
        // Note that this will override any serviceLocale variants.
        // For example, "sco-ulster-fonipa" + "...-fonupa" => "sco-fonupa" (replacing ulster).
        String variants = bestDesired.getVariant();
        if (!variants.isEmpty()) {
            b.setVariant(variants);
        }

        // Copy the extensions from bestDesired, if there are any.
        // Note that this will override any serviceLocale extensions.
        // For example, "th-u-nu-latn-ca-buddhist" + "...-u-nu-native" => "th-u-nu-native"
        // (replacing calendar).
        for (char extensionKey : bestDesired.getExtensionKeys()) {
            b.setExtension(extensionKey, bestDesired.getExtension(extensionKey));
        }
        serviceLocale = b.build();
    }
    return serviceLocale;
}
#endif

LocaleMatcher::Builder::Builder(LocaleMatcher::Builder &&src) U_NOEXCEPT :
        errorCode_(src.errorCode_),
        supportedLocales_(src.supportedLocales_),
        thresholdDistance_(src.thresholdDistance_),
        demotion_(src.demotion_),
        defaultLocale_(src.defaultLocale_),
        favor_(src.favor_) {
    src.supportedLocales_ = nullptr;
    src.defaultLocale_ = nullptr;
}

LocaleMatcher::Builder::~Builder() {
    delete supportedLocales_;
    delete defaultLocale_;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::operator=(LocaleMatcher::Builder &&src) U_NOEXCEPT {
    errorCode_ = src.errorCode_;
    supportedLocales_ = src.supportedLocales_;
    thresholdDistance_ = src.thresholdDistance_;
    demotion_ = src.demotion_;
    defaultLocale_ = src.defaultLocale_;
    favor_ = src.favor_;

    src.supportedLocales_ = nullptr;
    src.defaultLocale_ = nullptr;
    return *this;
}

void LocaleMatcher::Builder::clearSupportedLocales() {
    if (supportedLocales_ != nullptr) {
        supportedLocales_->removeAllElements();
    }
}

bool LocaleMatcher::Builder::ensureSupportedLocaleVector() {
    if (U_FAILURE(errorCode_)) { return false; }
    if (supportedLocales_ != nullptr) { return true; }
    supportedLocales_ = new UVector(uprv_deleteUObject, nullptr, errorCode_);
    if (U_FAILURE(errorCode_)) { return false; }
    if (supportedLocales_ == nullptr) {
        errorCode_ = U_MEMORY_ALLOCATION_ERROR;
        return false;
    }
    return true;
}

#if 0
LocaleMatcher::Builder &LocaleMatcher::Builder::setSupportedLocales(StringPiece locales) {
    if (U_FAILURE(errorCode_)) { return *this; }
    clearSupportedLocales();
    if (!ensureSupportedLocaleVector()) { return *this; }
    return setSupportedULocales(LocalePriorityList.add(locales).build().getULocales());
}
#endif

LocaleMatcher::Builder &LocaleMatcher::Builder::setSupportedLocales(Locale::Iterator &locales) {
    if (U_FAILURE(errorCode_)) { return *this; }
    clearSupportedLocales();
    if (!ensureSupportedLocaleVector()) { return *this; }
    while (locales.hasNext()) {
        const Locale &locale = locales.next();
        Locale *clone = locale.clone();
        if (clone == nullptr) {
            errorCode_ = U_MEMORY_ALLOCATION_ERROR;
            break;
        }
        supportedLocales_->addElement(clone, errorCode_);
        if (U_FAILURE(errorCode_)) {
            delete clone;
            break;
        }
    }
    return *this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::addSupportedLocale(const Locale &locale) {
    if (U_FAILURE(errorCode_)) { return *this; }
    if (!ensureSupportedLocaleVector()) { return *this; }
    Locale *clone = locale.clone();
    if (clone == nullptr) {
        errorCode_ = U_MEMORY_ALLOCATION_ERROR;
        return *this;
    }
    supportedLocales_->addElement(clone, errorCode_);
    if (U_FAILURE(errorCode_)) {
        delete clone;
    }
    return *this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setDefaultLocale(const Locale *defaultLocale) {
    if (U_FAILURE(errorCode_)) { return *this; }
    Locale *clone = nullptr;
    if (defaultLocale != nullptr) {
        clone = defaultLocale->clone();
        if (clone == nullptr) {
            errorCode_ = U_MEMORY_ALLOCATION_ERROR;
            return *this;
        }
    }
    delete defaultLocale_;
    defaultLocale_ = clone;
    return *this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setFavorSubtag(ULocMatchFavorSubtag subtag) {
    if (U_FAILURE(errorCode_)) { return *this; }
    favor_ = subtag;
    return *this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setDemotionPerDesiredLocale(ULocMatchDemotion demotion) {
    if (U_FAILURE(errorCode_)) { return *this; }
    demotion_ = demotion;
    return *this;
}

#if 0
/**
 * <i>Internal only!</i>
 *
 * @param thresholdDistance the thresholdDistance to set, with -1 = default
 * @return this Builder object
 * @internal
 * @deprecated This API is ICU internal only.
 */
@Deprecated
LocaleMatcher::Builder &LocaleMatcher::Builder::internalSetThresholdDistance(int32_t thresholdDistance) {
    if (U_FAILURE(errorCode_)) { return *this; }
    if (thresholdDistance > 100) {
        thresholdDistance = 100;
    }
    thresholdDistance_ = thresholdDistance;
    return *this;
}
#endif

UBool LocaleMatcher::Builder::copyErrorTo(UErrorCode &outErrorCode) const {
    if (U_FAILURE(outErrorCode)) { return TRUE; }
    if (U_SUCCESS(errorCode_)) { return FALSE; }
    outErrorCode = errorCode_;
    return TRUE;
}

LocaleMatcher LocaleMatcher::Builder::build(UErrorCode &errorCode) const {
    if (U_SUCCESS(errorCode) && U_FAILURE(errorCode_)) {
        errorCode = errorCode_;
    }
    return LocaleMatcher(*this, errorCode);
}

namespace {

LSR getMaximalLsrOrUnd(const XLikelySubtags &likelySubtags, const Locale &locale,
                       UErrorCode &errorCode) {
    if (U_FAILURE(errorCode) || locale.isBogus() || *locale.getName() == 0 /* "und" */) {
        return UND_LSR;
    } else {
        return likelySubtags.makeMaximizedLsrFrom(locale, errorCode);
    }
}

int32_t hashLSR(const UHashTok token) {
    const LSR *lsr = static_cast<const LSR *>(token.pointer);
    return (ustr_hashCharsN(lsr->language, lsr->languageLength) * 37 +
            ustr_hashCharsN(lsr->script, lsr->scriptLength)) * 37 +
            lsr->regionIndex;
}

UBool compareLSRs(const UHashTok t1, const UHashTok t2) {
    const LSR *lsr1 = static_cast<const LSR *>(t1.pointer);
    const LSR *lsr2 = static_cast<const LSR *>(t2.pointer);
    return *lsr1 == *lsr2;
}

bool putIfAbsent(UHashtable *lsrToIndex, const LSR &lsr, int32_t i, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode)) { return false; }
    U_ASSERT(i > 0);
    int32_t index = uhash_geti(lsrToIndex, &lsr);
    if (index != 0) {
        return false;
    } else {
        uhash_puti(lsrToIndex, const_cast<LSR *>(&lsr), i, &errorCode);
        return U_SUCCESS(errorCode);
    }
}

}  // namespace

LocaleMatcher::LocaleMatcher(const Builder &builder, UErrorCode &errorCode) :
        likelySubtags(*XLikelySubtags::getSingleton(errorCode)),
        localeDistance(*LocaleDistance::getSingleton(errorCode)),
        thresholdDistance(builder.thresholdDistance_),
        demotionPerDesiredLocale(builder.demotion_),
        favorSubtag(builder.favor_),
        supportedLocales(nullptr), lsrs(nullptr), supportedLocalesLength(0),
        supportedLsrToIndex(nullptr),
        supportedLsrs(nullptr), supportedIndexes(nullptr), supportedLsrsLength(0),
        ownedDefaultLocale(nullptr), defaultLocale(nullptr), defaultLocaleIndex(-1) {
    if (U_FAILURE(errorCode)) { return; }
    // TODO: begin remove
    Locale locale("zh-TW");
    LSR lsr = getMaximalLsrOrUnd(likelySubtags, locale, errorCode);
    printf("'%s' --> maximal '%s-%s-%s'\n", locale.getName(), lsr.language, lsr.script, lsr.region);
    // TODO: end remove

    thresholdDistance = builder.thresholdDistance_ < 0 ?
            localeDistance.getDefaultScriptDistance() : builder.thresholdDistance_;
    // Store the supported locales in input order,
    // so that when different types are used (e.g., language tag strings)
    // we can return those by parallel index.
    supportedLocalesLength = builder.supportedLocales_->size();
    if (supportedLocalesLength > 0) {
        supportedLocales = static_cast<const Locale **>(
            uprv_malloc(supportedLocalesLength * sizeof(const Locale *)));
        // Supported LRSs in input order.
        // In C++, we store these permanently to simplify ownership management
        // in the hash tables. Duplicate LSRs (if any) are unused overhead.
        lsrs = new LSR[supportedLocalesLength];
        if (supportedLocales == nullptr || lsrs == nullptr) {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
            return;
        }
    }
    // Also find the first supported locale whose LSR is
    // the same as that for the default locale.
    const Locale *def = builder.defaultLocale_;
    LSR builderDefaultLSR;
    const LSR *defLSR = nullptr;
    int32_t idef = -1;
    if (def != nullptr) {
        builderDefaultLSR = getMaximalLsrOrUnd(likelySubtags, *def, errorCode);
        if (U_FAILURE(errorCode)) { return; }
        defLSR = &builderDefaultLSR;
    }
    for (int32_t i = 0; i < supportedLocalesLength; ++i) {
        const Locale &locale = *static_cast<Locale *>(builder.supportedLocales_->elementAt(i));
        supportedLocales[i] = locale.clone();
        if (supportedLocales[i] == nullptr) {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
            return;
        }
        const Locale &supportedLocale = *supportedLocales[i];
        const LSR &lsr = lsrs[i] = getMaximalLsrOrUnd(likelySubtags, supportedLocale, errorCode);
        if (U_FAILURE(errorCode)) { return; }
        if (idef < 0 && defLSR != nullptr && lsr == *defLSR) {
            idef = i;
            defLSR = &lsr;  // owned pointer to put into supportedLsrToIndex
        }
    }
    // We need an unordered map from LSR to first supported locale with that LSR,
    // and an ordered list of (LSR, supported index).
    // We insert the supported locales in the following order:
    // 1. Default locale, if it is supported.
    // 2. Priority locales in builder order.
    // 3. Remaining locales in builder order.
    // In Java, we use a LinkedHashMap for both map & ordered lists.
    // In C++, we use separate structures.
    // We over-allocate lists of LSRs and indexes for simplicity.
    // We reserve slots at the array starts for the default and paradigm locales,
    // plus enough for all supported locales.
    // If there are few paradigm locales and few duplicate supported LSRs,
    // then the amount of wasted space is small.
    int32_t paradigmStart = 1;
    int32_t paradigmLimit = paradigmStart + localeDistance.getParadigmLSRsLength();
    int32_t maxSuppLsrs = paradigmLimit + supportedLocalesLength;
    if (supportedLocalesLength > 0) {
        supportedLsrToIndex = uhash_openSize(hashLSR, compareLSRs, uhash_compareLong,
                                             maxSuppLsrs, &errorCode);
        if (U_FAILURE(errorCode)) { return; }
        supportedLsrs = static_cast<const LSR **>(uprv_malloc(maxSuppLsrs * sizeof(const LSR *)));
        supportedIndexes = static_cast<int32_t *>(
            uprv_malloc(maxSuppLsrs * sizeof(int32_t)));
        if (supportedLsrs == nullptr || supportedIndexes == nullptr) {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
            return;
        }
    }
    int32_t paradigmIndex = paradigmStart;
    int32_t otherIndex = paradigmLimit;
    int32_t numSuppLsrs = 0;
    if (idef >= 0) {
        uhash_puti(supportedLsrToIndex, const_cast<LSR *>(defLSR), idef + 1, &errorCode);
        supportedLsrs[0] = defLSR;
        supportedIndexes[0] = idef;
        ++numSuppLsrs;
    }
    for (int32_t i = 0; i < supportedLocalesLength; ++i) {
        if (i == idef) { continue; }
        const Locale &locale = *supportedLocales[i];
        const LSR &lsr = lsrs[i];
        if (defLSR == nullptr) {
            U_ASSERT(i == 0);
            def = &locale;
            defLSR = &lsr;
            idef = 0;
            uhash_puti(supportedLsrToIndex, const_cast<LSR *>(&lsr), 0 + 1, &errorCode);
            supportedLsrs[0] = &lsr;
            supportedIndexes[0] = 0;
            ++numSuppLsrs;
        } else if (lsr == *defLSR) {
            // already in the map
        } else {
            if (putIfAbsent(supportedLsrToIndex, lsr, i + 1, errorCode)) {
                if (localeDistance.isParadigmLSR(lsr)) {
                    supportedLsrs[paradigmIndex] = &lsr;
                    supportedIndexes[paradigmIndex] = i;
                    ++paradigmIndex;
                } else {
                    supportedLsrs[otherIndex] = &lsr;
                    supportedIndexes[otherIndex] = i;
                    ++otherIndex;
                }
                ++numSuppLsrs;
            }
        }
        if (U_FAILURE(errorCode)) { return; }
    }
    // Squeeze out unused array slots.
    if (paradigmIndex < paradigmLimit && paradigmLimit < otherIndex) {
        uprv_memmove(supportedLsrs + paradigmIndex, supportedLsrs + paradigmLimit,
                     (otherIndex - paradigmLimit) * sizeof(const LSR *));
        uprv_memmove(supportedIndexes + paradigmIndex, supportedIndexes + paradigmLimit,
                     (otherIndex - paradigmLimit) * sizeof(int32_t));
    }
    supportedLsrsLength = numSuppLsrs;

    if (def != nullptr && (idef < 0 || *def != *supportedLocales[idef])) {
        ownedDefaultLocale = def->clone();
        if (ownedDefaultLocale == nullptr) {
            errorCode = U_MEMORY_ALLOCATION_ERROR;
            return;
        }
        def = ownedDefaultLocale;
    }
    defaultLocale = def;
    defaultLocaleIndex = idef;

    demotionPerDesiredLocale =
            builder.demotion_ == ULOCMATCH_DEMOTION_NONE ? 0 :
                localeDistance.getDefaultDemotionPerDesiredLocale();  // REGION
    favorSubtag = builder.favor_;
}

LocaleMatcher::LocaleMatcher(LocaleMatcher &&src) U_NOEXCEPT :
        likelySubtags(src.likelySubtags),
        localeDistance(src.localeDistance),
        thresholdDistance(src.thresholdDistance),
        demotionPerDesiredLocale(src.demotionPerDesiredLocale),
        favorSubtag(src.favorSubtag),
        supportedLocales(src.supportedLocales), lsrs(src.lsrs),
        supportedLocalesLength(src.supportedLocalesLength),
        supportedLsrToIndex(src.supportedLsrToIndex),
        supportedLsrs(src.supportedLsrs),
        supportedIndexes(src.supportedIndexes),
        supportedLsrsLength(src.supportedLsrsLength),
        ownedDefaultLocale(src.ownedDefaultLocale), defaultLocale(src.defaultLocale),
        defaultLocaleIndex(src.defaultLocaleIndex) {
    src.supportedLocales = nullptr;
    src.lsrs = nullptr;
    src.supportedLocalesLength = 0;
    src.supportedLsrToIndex = nullptr;
    src.supportedLsrs = nullptr;
    src.supportedIndexes = nullptr;
    src.supportedLsrsLength = 0;
    src.ownedDefaultLocale = nullptr;
    src.defaultLocale = nullptr;
    src.defaultLocaleIndex = -1;
}

LocaleMatcher::~LocaleMatcher() {
    for (int32_t i = 0; i < supportedLocalesLength; ++i) {
        delete supportedLocales[i];
    }
    uprv_free(supportedLocales);
    delete[] lsrs;
    uhash_close(supportedLsrToIndex);
    uprv_free(supportedLsrs);
    uprv_free(supportedIndexes);
    delete ownedDefaultLocale;
}

LocaleMatcher &LocaleMatcher::operator=(LocaleMatcher &&src) U_NOEXCEPT {
    thresholdDistance = src.thresholdDistance;
    demotionPerDesiredLocale = src.demotionPerDesiredLocale;
    favorSubtag = src.favorSubtag;
    supportedLocales = src.supportedLocales;
    lsrs = src.lsrs;
    supportedLocalesLength = src.supportedLocalesLength;
    supportedLsrToIndex = src.supportedLsrToIndex;
    supportedLsrs = src.supportedLsrs;
    supportedIndexes = src.supportedIndexes;
    supportedLsrsLength = src.supportedLsrsLength;
    ownedDefaultLocale = src.ownedDefaultLocale;
    defaultLocale = src.defaultLocale;
    defaultLocaleIndex = src.defaultLocaleIndex;

    src.supportedLocales = nullptr;
    src.lsrs = nullptr;
    src.supportedLocalesLength = 0;
    src.supportedLsrToIndex = nullptr;
    src.supportedLsrs = nullptr;
    src.supportedIndexes = nullptr;
    src.supportedLsrsLength = 0;
    src.ownedDefaultLocale = nullptr;
    src.defaultLocale = nullptr;
    src.defaultLocaleIndex = -1;
    return *this;
}

class LocaleLsrIterator {
public:
    LocaleLsrIterator(const XLikelySubtags &likelySubtags, Locale::Iterator &locales,
                      ULocMatchLifetime lifetime) :
            likelySubtags(likelySubtags), locales(locales), lifetime(lifetime) {}

    ~LocaleLsrIterator() {
        if (lifetime == ULOCMATCH_TEMPORARY_LOCALES) {
            delete remembered;
        }
    }

    bool hasNext() const {
        return locales.hasNext();
    }

    LSR next(UErrorCode &errorCode) {
        current = &locales.next();
        return getMaximalLsrOrUnd(likelySubtags, *current, errorCode);
    }

    void rememberCurrent(int32_t desiredIndex, UErrorCode &errorCode) {
        if (U_FAILURE(errorCode)) { return; }
        bestDesiredIndex = desiredIndex;
        if (lifetime == ULOCMATCH_STORED_LOCALES) {
            remembered = current;
        } else {
            // ULOCMATCH_TEMPORARY_LOCALES
            delete remembered;
            remembered = new Locale(*current);
            if (remembered == nullptr) {
                errorCode = U_MEMORY_ALLOCATION_ERROR;
            }
        }
    }

    const Locale *orphanRemembered() {
        const Locale *rem = remembered;
        remembered = nullptr;
        return rem;
    }

    int32_t getBestDesiredIndex() const {
        return bestDesiredIndex;
    }

private:
    const XLikelySubtags &likelySubtags;
    Locale::Iterator &locales;
    ULocMatchLifetime lifetime;
    const Locale *current = nullptr, *remembered = nullptr;
    int32_t bestDesiredIndex = -1;
};

const Locale *LocaleMatcher::getBestMatch(const Locale &desiredLocale, UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode)) { return nullptr; }
    int32_t suppIndex = getBestSuppIndex(
        getMaximalLsrOrUnd(likelySubtags, desiredLocale, errorCode),
        nullptr, errorCode);
    return U_SUCCESS(errorCode) && suppIndex >= 0 ? supportedLocales[suppIndex] : defaultLocale;
}

const Locale *LocaleMatcher::getBestMatch(Locale::Iterator &desiredLocales,
                                          UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode)) { return nullptr; }
    if (!desiredLocales.hasNext()) {
        return defaultLocale;
    }
    LocaleLsrIterator lsrIter(likelySubtags, desiredLocales, ULOCMATCH_TEMPORARY_LOCALES);
    int32_t suppIndex = getBestSuppIndex(lsrIter.next(errorCode), &lsrIter, errorCode);
    return U_SUCCESS(errorCode) && suppIndex >= 0 ? supportedLocales[suppIndex] : defaultLocale;
}

#if 0
const Locale *LocaleMatcher::getBestMatch(StringPiece desiredLocaleList, UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode)) { return nullptr; }
    return getBestMatch(LocalePriorityList.add(desiredLocaleList).build());
}
#endif

LocaleMatcher::Result LocaleMatcher::getBestMatchResult(
        const Locale &desiredLocale, UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode)) {
        return Result(nullptr, defaultLocale, -1, defaultLocaleIndex, FALSE);
    }
    int32_t suppIndex = getBestSuppIndex(
        getMaximalLsrOrUnd(likelySubtags, desiredLocale, errorCode),
        nullptr, errorCode);
    if (U_FAILURE(errorCode) || suppIndex < 0) {
        return Result(nullptr, defaultLocale, -1, defaultLocaleIndex, FALSE);
    } else {
        return Result(&desiredLocale, supportedLocales[suppIndex], 0, suppIndex, FALSE);
    }
}

LocaleMatcher::Result LocaleMatcher::getBestMatchResult(
        Locale::Iterator &desiredLocales, UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode) || !desiredLocales.hasNext()) {
        return Result(nullptr, defaultLocale, -1, defaultLocaleIndex, FALSE);
    }
    LocaleLsrIterator lsrIter(likelySubtags, desiredLocales, ULOCMATCH_TEMPORARY_LOCALES);
    int32_t suppIndex = getBestSuppIndex(lsrIter.next(errorCode), &lsrIter, errorCode);
    if (U_FAILURE(errorCode) || suppIndex < 0) {
        return Result(nullptr, defaultLocale, -1, defaultLocaleIndex, FALSE);
    } else {
        return Result(lsrIter.orphanRemembered(), supportedLocales[suppIndex],
                      lsrIter.getBestDesiredIndex(), suppIndex, TRUE);
    }
}

int32_t LocaleMatcher::getBestSuppIndex(LSR desiredLSR, LocaleLsrIterator *remainingIter,
                                        UErrorCode &errorCode) const {
    if (U_FAILURE(errorCode)) { return -1; }
    int32_t desiredIndex = 0;
    int32_t bestSupportedLsrIndex = -1;
    for (int32_t bestDistance = thresholdDistance;;) {
        // Quick check for exact maximized LSR.
        // Returns suppIndex+1 where 0 means not found.
        int32_t index = uhash_geti(supportedLsrToIndex, &desiredLSR);
        if (index != 0) {
            int32_t suppIndex = index - 1;
            if (remainingIter != nullptr) {
                remainingIter->rememberCurrent(desiredIndex, errorCode);
            }
            return suppIndex;
        }
        int32_t bestIndexAndDistance = localeDistance.getBestIndexAndDistance(
                desiredLSR, supportedLsrs, supportedLsrsLength, bestDistance, favorSubtag);
        if (bestIndexAndDistance >= 0) {
            bestDistance = bestIndexAndDistance & 0xff;
            if (remainingIter != nullptr) {
                remainingIter->rememberCurrent(desiredIndex, errorCode);
                if (U_FAILURE(errorCode)) { return -1; }
            }
            bestSupportedLsrIndex = bestIndexAndDistance >= 0 ? bestIndexAndDistance >> 8 : -1;
        }
        if ((bestDistance -= demotionPerDesiredLocale) <= 0) {
            break;
        }
        if (remainingIter == nullptr || !remainingIter->hasNext()) {
            break;
        }
        desiredLSR = remainingIter->next(errorCode);
        if (U_FAILURE(errorCode)) { return -1; }
    }
    if (bestSupportedLsrIndex < 0) {
        // no good match
        return -1;
    }
    return supportedIndexes[bestSupportedLsrIndex];
}
#if 0
double LocaleMatcher::internalMatch(const Locale &desired, const Locale &supported, UErrorCode &errorCode) {
    // Returns the inverse of the distance: That is, 1-distance(desired, supported).
    int32_t distance = localeDistance.getBestIndexAndDistance(
            likelySubtags.makeMaximizedLsrFrom(desired),
            new LSR[] { likelySubtags.makeMaximizedLsrFrom(supported) },
            thresholdDistance, favorSubtag) & 0xff;
    return (100 - distance) / 100.0;
}
#endif
U_NAMESPACE_END

#endif  // __LOCMATCHER_H__
