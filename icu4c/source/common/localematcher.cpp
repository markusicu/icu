// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// locmatcher.h
// created: 2019may08 Markus W. Scherer

#ifndef __LOCMATCHER_H__
#define __LOCMATCHER_H__

#include "unicode/utypes.h"
#include "unicode/localematcher.h"
#include "unicode/locid.h"
#include "unicode/uobject.h"

U_NAMESPACE_BEGIN

#if 0

// TODO class U_COMMON_API LocaleMatcher final : public UMemory {
private static final LSR UND_LSR = new LSR("und","","");
private static final ULocale UND_ULOCALE = new ULocale("und");
private static final Locale UND_LOCALE = new Locale("und");

private static abstract class LsrIterator implements Iterator<LSR> {
    int bestDesiredIndex = -1;

    public abstract void rememberCurrent(int desiredIndex);
}

Locale LocaleMatcher::Result::makeServiceLocale() {
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

LocaleMatcher::Builder &LocaleMatcher::Builder::setSupportedLocales(StringPiece locales) {
    return setSupportedULocales(LocalePriorityList.add(locales).build().getULocales());
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setSupportedLocales(Locale::Iterator &locales) {
    supportedLocales = new ArrayList<>(locales.size());
    for (Locale locale : locales) {
        supportedLocales.add(ULocale.forLocale(locale));
    }
    return this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::addSupportedLocale(const Locale &locale) {
    if (supportedLocales == null) {
        supportedLocales = new ArrayList<>();
    }
    supportedLocales.add(locale);
    return this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setDefaultLocale(const Locale &defaultLocale) {
    this.defaultLocale = defaultLocale;
    return this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setFavorSubtag(ULocMatchFavorSubtag subtag) {
    this.favor = subtag;
    return this;
}

LocaleMatcher::Builder &LocaleMatcher::Builder::setDemotionPerDesiredLocale(ULocMatchDemotion demotion) {
    this.demotion = demotion;
    return this;
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
LocaleMatcher::Builder LocaleMatcher::Builder::internalSetThresholdDistance(int thresholdDistance) {
    if (thresholdDistance > 100) {
        thresholdDistance = 100;
    }
    this.thresholdDistance = thresholdDistance;
    return this;
}
#endif

UBool LocaleMatcher::Builder::copyErrorTo(UErrorCode &outErrorCode);

LocaleMatcher LocaleMatcher::Builder::build(UErrorCode &errorCode) {
    return new LocaleMatcher(this);
}

private LocaleMatcher(const Builder &builder, UErrorCode &errorCode) {
    thresholdDistance = builder.thresholdDistance < 0 ?
            LocaleDistance.INSTANCE.getDefaultScriptDistance() : builder.thresholdDistance;
    // Store the supported locales in input order,
    // so that when different types are used (e.g., java.util.Locale)
    // we can return those by parallel index.
    int supportedLocalesLength = builder.supportedLocales.size();
    supportedULocales = new ULocale[supportedLocalesLength];
    supportedLocales = new Locale[supportedLocalesLength];
    // Supported LRSs in input order.
    LSR lsrs[] = new LSR[supportedLocalesLength];
    // Also find the first supported locale whose LSR is
    // the same as that for the default locale.
    ULocale udef = builder.defaultLocale;
    Locale def = null;
    LSR defLSR = null;
    int idef = -1;
    if (udef != null) {
        def = udef.toLocale();
        defLSR = getMaximalLsrOrUnd(udef);
    }
    int i = 0;
    for (ULocale locale : builder.supportedLocales) {
        supportedULocales[i] = locale;
        supportedLocales[i] = locale.toLocale();
        LSR lsr = lsrs[i] = getMaximalLsrOrUnd(locale);
        if (idef < 0 && defLSR != null && lsr.equals(defLSR)) {
            idef = i;
        }
        ++i;
    }

    // We need an unordered map from LSR to first supported locale with that LSR,
    // and an ordered list of (LSR, Indexes).
    // We use a LinkedHashMap for both,
    // and insert the supported locales in the following order:
    // 1. Default locale, if it is supported.
    // 2. Priority locales in builder order.
    // 3. Remaining locales in builder order.
    supportedLsrToIndex = new LinkedHashMap<>(supportedLocalesLength);
    Map<LSR, Integer> otherLsrToIndex = null;
    if (idef >= 0) {
        supportedLsrToIndex.put(defLSR, idef);
    }
    i = 0;
    for (ULocale locale : supportedULocales) {
        if (i == idef) { continue; }
        LSR lsr = lsrs[i];
        if (defLSR == null) {
            assert i == 0;
            udef = locale;
            def = supportedLocales[0];
            defLSR = lsr;
            idef = 0;
            supportedLsrToIndex.put(lsr, 0);
        } else if (lsr.equals(defLSR) || LocaleDistance.INSTANCE.isParadigmLSR(lsr)) {
            putIfAbsent(supportedLsrToIndex, lsr, i);
        } else {
            if (otherLsrToIndex == null) {
                otherLsrToIndex = new LinkedHashMap<>(supportedLocalesLength);
            }
            putIfAbsent(otherLsrToIndex, lsr, i);
        }
        ++i;
    }
    if (otherLsrToIndex != null) {
        supportedLsrToIndex.putAll(otherLsrToIndex);
    }
    int numSuppLsrs = supportedLsrToIndex.size();
    supportedLsrs = new LSR[numSuppLsrs];
    supportedIndexes = new int[numSuppLsrs];
    i = 0;
    for (Map.Entry<LSR, Integer> entry : supportedLsrToIndex.entrySet()) {
        supportedLsrs[i] = entry.getKey();  // = lsrs[entry.getValue()]
        supportedIndexes[i++] = entry.getValue();
    }

    defaultULocale = udef;
    defaultLocale = def;
    defaultLocaleIndex = idef;
    demotionPerDesiredLocale =
            builder.demotion == Demotion.NONE ? 0 :
                LocaleDistance.INSTANCE.getDefaultDemotionPerDesiredLocale();  // null or REGION
    favorSubtag = builder.favor;
}

private static final void putIfAbsent(Map<LSR, Integer> lsrToIndex, LSR lsr, int i) {
    Integer index = lsrToIndex.get(lsr);
    if (index == null) {
        lsrToIndex.put(lsr, i);
    }
}

private static final LSR getMaximalLsrOrUnd(ULocale locale) {
    if (locale.equals(UND_ULOCALE)) {
        return UND_LSR;
    } else {
        return XLikelySubtags.INSTANCE.makeMaximizedLsrFrom(locale);
    }
}

private static final LSR getMaximalLsrOrUnd(Locale locale) {
    if (locale.equals(UND_LOCALE)) {
        return UND_LSR;
    } else {
        return XLikelySubtags.INSTANCE.makeMaximizedLsrFrom(locale);
    }
}

private static final class ULocaleLsrIterator extends LsrIterator {
    private Iterator<ULocale> locales;
    private ULocale current, remembered;

    ULocaleLsrIterator(Iterator<ULocale> locales) {
        this.locales = locales;
    }

    @Override
    public boolean hasNext() {
        return locales.hasNext();
    }

    @Override
    public LSR next() {
        current = locales.next();
        return getMaximalLsrOrUnd(current);
    }

    @Override
    public void rememberCurrent(int desiredIndex) {
        bestDesiredIndex = desiredIndex;
        remembered = current;
    }
}

private static final class LocaleLsrIterator extends LsrIterator {
    private Iterator<Locale> locales;
    private Locale current, remembered;

    LocaleLsrIterator(Iterator<Locale> locales) {
        this.locales = locales;
    }

    @Override
    public boolean hasNext() {
        return locales.hasNext();
    }

    @Override
    public LSR next() {
        current = locales.next();
        return getMaximalLsrOrUnd(current);
    }

    @Override
    public void rememberCurrent(int desiredIndex) {
        bestDesiredIndex = desiredIndex;
        remembered = current;
    }
}

Locale *getBestMatch(const Locale &desiredLocale, UErrorCode &errorCode) {
    LSR desiredLSR = getMaximalLsrOrUnd(desiredLocale);
    int suppIndex = getBestSuppIndex(desiredLSR, null);
    return suppIndex >= 0 ? supportedULocales[suppIndex] : defaultULocale;
}

Locale *getBestMatch(Locale::Iterator &desiredLocales, UErrorCode &errorCode) {
    Iterator<ULocale> desiredIter = desiredLocales.iterator();
    if (!desiredIter.hasNext()) {
        return defaultULocale;
    }
    ULocaleLsrIterator lsrIter = new ULocaleLsrIterator(desiredIter);
    LSR desiredLSR = lsrIter.next();
    int suppIndex = getBestSuppIndex(desiredLSR, lsrIter);
    return suppIndex >= 0 ? supportedULocales[suppIndex] : defaultULocale;
}

Locale *getBestMatch(StringPiece desiredLocaleList, UErrorCode &errorCode) {
    return getBestMatch(LocalePriorityList.add(desiredLocaleList).build());
}

private Result makeResult(Locale desiredLocale, LocaleLsrIterator lsrIter, int suppIndex) {
    if (suppIndex < 0) {
        return new Result(null, defaultULocale, null, defaultLocale, -1, defaultLocaleIndex);
    } else if (desiredLocale != null) {
        return new Result(null, supportedULocales[suppIndex],
                desiredLocale, supportedLocales[suppIndex], 0, suppIndex);
    } else {
        return new Result(null, supportedULocales[suppIndex],
                lsrIter.remembered, supportedLocales[suppIndex],
                lsrIter.bestDesiredIndex, suppIndex);
    }
}

Result getBestMatchResult(const Locale &desiredLocale, UErrorCode &errorCode) {
    LSR desiredLSR = getMaximalLsrOrUnd(desiredLocale);
    int suppIndex = getBestSuppIndex(desiredLSR, null);
    return makeResult(desiredLocale, null, suppIndex);
}

Result getBestMatchResult(Locale::Iterator &desiredLocales, ULocMatchLifetime lifetime,
                          UErrorCode &errorCode) {
    Iterator<ULocale> desiredIter = desiredLocales.iterator();
    if (!desiredIter.hasNext()) {
        return makeResult(UND_ULOCALE, null, -1);
    }
    ULocaleLsrIterator lsrIter = new ULocaleLsrIterator(desiredIter);
    LSR desiredLSR = lsrIter.next();
    int suppIndex = getBestSuppIndex(desiredLSR, lsrIter);
    return makeResult(null, lsrIter, suppIndex);
}

private int getBestSuppIndex(LSR desiredLSR, LsrIterator remainingIter) {
    int desiredIndex = 0;
    int bestSupportedLsrIndex = -1;
    for (int bestDistance = thresholdDistance;;) {
        // Quick check for exact maximized LSR.
        Integer index = supportedLsrToIndex.get(desiredLSR);
        if (index != null) {
            int suppIndex = index;
            if (TRACE_MATCHER) {
                System.err.printf("Returning %s: desiredLSR=supportedLSR\n",
                        supportedULocales[suppIndex]);
            }
            if (remainingIter != null) { remainingIter.rememberCurrent(desiredIndex); }
            return suppIndex;
        }
        int bestIndexAndDistance = LocaleDistance.INSTANCE.getBestIndexAndDistance(
                desiredLSR, supportedLsrs, bestDistance, favorSubtag);
        if (bestIndexAndDistance >= 0) {
            bestDistance = bestIndexAndDistance & 0xff;
            if (remainingIter != null) { remainingIter.rememberCurrent(desiredIndex); }
            bestSupportedLsrIndex = bestIndexAndDistance >> 8;
        }
        if ((bestDistance -= demotionPerDesiredLocale) <= 0) {
            break;
        }
        if (remainingIter == null || !remainingIter.hasNext()) {
            break;
        }
        desiredLSR = remainingIter.next();
    }
    if (bestSupportedLsrIndex < 0) {
        if (TRACE_MATCHER) {
            System.err.printf("Returning default %s: no good match\n", defaultULocale);
        }
        return -1;
    }
    int suppIndex = supportedIndexes[bestSupportedLsrIndex];
    if (TRACE_MATCHER) {
        System.err.printf("Returning %s: best matching supported locale\n",
                supportedULocales[suppIndex]);
    }
    return suppIndex;
}

double LocaleMatcher::internalMatch(const Locale &desired, const Locale &supported, UErrorCode &errorCode) {
    // Returns the inverse of the distance: That is, 1-distance(desired, supported).
    int distance = LocaleDistance.INSTANCE.getBestIndexAndDistance(
            XLikelySubtags.INSTANCE.makeMaximizedLsrFrom(desired),
            new LSR[] { XLikelySubtags.INSTANCE.makeMaximizedLsrFrom(supported) },
            thresholdDistance, favorSubtag) & 0xff;
    return (100 - distance) / 100.0;
}

#endif

U_NAMESPACE_END

#endif  // __LOCMATCHER_H__
