// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// localematchertest.cpp
// created: 2019jul04 Markus W. Scherer

#include <stdio.h>  // TODO
#include <string>
#include <vector>

#include "unicode/utypes.h"
#include "unicode/localematcher.h"
#include "unicode/locid.h"
#include "cmemory.h"
#include "intltest.h"

#define ARRAY_RANGE(array) (array), ((array) + UPRV_LENGTHOF(array))

namespace {

const char *locString(const Locale *loc) {
    return loc != nullptr ? loc->getName() : "(null)";
}

}  // namespace

class LocaleMatcherTest : public IntlTest {
public:
    LocaleMatcherTest() {}

    void runIndexedTest(int32_t index, UBool exec, const char *&name, char *par=NULL);

    void testEmpty();
    void testBasics();
    void testSupportedDefault();
    void testUnsupportedDefault();
    void testDemotion();
    void testMatch();
    void testResolvedLocale();
};

extern IntlTest *createLocaleMatcherTest() {
    return new LocaleMatcherTest();
}

void LocaleMatcherTest::runIndexedTest(int32_t index, UBool exec, const char *&name, char * /*par*/) {
    if(exec) {
        logln("TestSuite LocaleMatcherTest: ");
    }
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO(testEmpty);
    TESTCASE_AUTO(testBasics);
    TESTCASE_AUTO(testSupportedDefault);
    TESTCASE_AUTO(testUnsupportedDefault);
    TESTCASE_AUTO(testDemotion);
    TESTCASE_AUTO(testMatch);
    TESTCASE_AUTO(testResolvedLocale);
    TESTCASE_AUTO_END;
}

void LocaleMatcherTest::testEmpty() {
    IcuTestErrorCode errorCode(*this, "testEmpty");
    LocaleMatcher matcher = LocaleMatcher::Builder().build(errorCode);
    const Locale *best = matcher.getBestMatch(Locale::getFrench(), errorCode);
    assertEquals("getBestMatch(fr)", "(null)", locString(best));
    LocaleMatcher::Result result = matcher.getBestMatchResult("fr", errorCode);
    assertEquals("getBestMatchResult(fr).supp",
                 "(null)", locString(result.getSupportedLocale()));
    assertEquals("getBestMatchResult(fr).suppIndex",
                 -1, result.getSupportedIndex());
}

void LocaleMatcherTest::testBasics() {
    IcuTestErrorCode errorCode(*this, "testBasics");
    Locale locales[] = { "fr", "en_GB", "en" };
    {
        LocaleMatcher matcher = LocaleMatcher::Builder().
            setSupportedLocales(ARRAY_RANGE(locales)).build(errorCode);
        const Locale *best = matcher.getBestMatch("en_GB", errorCode);
        assertEquals("fromRange.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("fromRange.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("fromRange.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("fromRange.getBestMatch(ja_JP)", "fr", locString(best));
    }
    // Code coverage: Variations of setting supported locales.
    // TODO: Unit-test Locale::*Iterator directly, in a class-Locale test file.
    {
        std::vector<Locale> locales{ "fr", "en_GB", "en" };
        LocaleMatcher matcher = LocaleMatcher::Builder().
            setSupportedLocales(locales.begin(), locales.end()).build(errorCode);
        const Locale *best = matcher.getBestMatch("en_GB", errorCode);
        assertEquals("fromRange.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("fromRange.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("fromRange.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("fromRange.getBestMatch(ja_JP)", "fr", locString(best));
    }
    {
        Locale::RangeIterator<Locale *> iter(ARRAY_RANGE(locales));
        LocaleMatcher matcher = LocaleMatcher::Builder().
            setSupportedLocales(iter).build(errorCode);
        const Locale *best = matcher.getBestMatch("en_GB", errorCode);
        assertEquals("fromIter.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("fromIter.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("fromIter.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("fromIter.getBestMatch(ja_JP)", "fr", locString(best));
    }
    {
        Locale *pointers[] = { locales, locales + 1, locales + 2 };
        LocaleMatcher matcher = LocaleMatcher::Builder().
            setSupportedLocalesViaConverter(
                ARRAY_RANGE(pointers), [](const Locale *p) { return *p; }).
            build(errorCode);
        const Locale *best = matcher.getBestMatch("en_GB", errorCode);
        assertEquals("viaConverter.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("viaConverter.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("viaConverter.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("viaConverter.getBestMatch(ja_JP)", "fr", locString(best));
    }
    {
        LocaleMatcher matcher = LocaleMatcher::Builder().
            addSupportedLocale(locales[0]).
            addSupportedLocale(locales[1]).
            addSupportedLocale(locales[2]).
            build(errorCode);
        const Locale *best = matcher.getBestMatch("en_GB", errorCode);
        assertEquals("added.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("added.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("added.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("added.getBestMatch(ja_JP)", "fr", locString(best));
    }
    {
        LocaleMatcher matcher = LocaleMatcher::Builder().
            setSupportedLocalesFromListString(
                " el, fr;q=0.555555, en-GB ; q = 0.88  , el; q =0, en;q=0.88 , fr ").
            build(errorCode);
        const Locale *best = matcher.getBestMatchForListString("el, fr, fr;q=0, en-GB", errorCode);
        assertEquals("added.getBestMatch(en_GB)", "en_GB", locString(best));
        best = matcher.getBestMatch("en_US", errorCode);
        assertEquals("added.getBestMatch(en_US)", "en", locString(best));
        best = matcher.getBestMatch("fr_FR", errorCode);
        assertEquals("added.getBestMatch(fr_FR)", "fr", locString(best));
        best = matcher.getBestMatch("ja_JP", errorCode);
        assertEquals("added.getBestMatch(ja_JP)", "fr", locString(best));
    }
}

void LocaleMatcherTest::testSupportedDefault() {
    // The default locale is one of the supported locales.
    IcuTestErrorCode errorCode(*this, "testSupportedDefault");
    Locale locales[] = { "fr", "en_GB", "en" };
    LocaleMatcher matcher = LocaleMatcher::Builder().
        setSupportedLocales(ARRAY_RANGE(locales)).
        setDefaultLocale(&locales[1]).
        build(errorCode);
    const Locale *best = matcher.getBestMatch("en_GB", errorCode);
    assertEquals("getBestMatch(en_GB)", "en_GB", locString(best));
    best = matcher.getBestMatch("en_US", errorCode);
    assertEquals("getBestMatch(en_US)", "en", locString(best));
    best = matcher.getBestMatch("fr_FR", errorCode);
    assertEquals("getBestMatch(fr_FR)", "fr", locString(best));
    best = matcher.getBestMatch("ja_JP", errorCode);
    assertEquals("getBestMatch(ja_JP)", "en_GB", locString(best));
    LocaleMatcher::Result result = matcher.getBestMatchResult("ja_JP", errorCode);
    assertEquals("getBestMatchResult(ja_JP).supp",
                 "en_GB", locString(result.getSupportedLocale()));
    assertEquals("getBestMatchResult(ja_JP).suppIndex",
                 1, result.getSupportedIndex());
}

void LocaleMatcherTest::testUnsupportedDefault() {
    // The default locale does not match any of the supported locales.
    IcuTestErrorCode errorCode(*this, "testSupportedDefault");
    Locale locales[] = { "fr", "en_GB", "en" };
    Locale def("de");
    LocaleMatcher matcher = LocaleMatcher::Builder().
        setSupportedLocales(ARRAY_RANGE(locales)).
        setDefaultLocale(&def).
        build(errorCode);
    const Locale *best = matcher.getBestMatch("en_GB", errorCode);
    assertEquals("getBestMatch(en_GB)", "en_GB", locString(best));
    best = matcher.getBestMatch("en_US", errorCode);
    assertEquals("getBestMatch(en_US)", "en", locString(best));
    best = matcher.getBestMatch("fr_FR", errorCode);
    assertEquals("getBestMatch(fr_FR)", "fr", locString(best));
    best = matcher.getBestMatch("ja_JP", errorCode);
    assertEquals("getBestMatch(ja_JP)", "de", locString(best));
    LocaleMatcher::Result result = matcher.getBestMatchResult("ja_JP", errorCode);
    assertEquals("getBestMatchResult(ja_JP).supp",
                 "de", locString(result.getSupportedLocale()));
    assertEquals("getBestMatchResult(ja_JP).suppIndex",
                 -1, result.getSupportedIndex());
}

void LocaleMatcherTest::testDemotion() {
    IcuTestErrorCode errorCode(*this, "testDemotion");
    Locale supported[] = { "fr", "de-CH", "it" };
    Locale desired[] = { "fr-CH", "de-CH", "it" };
    {
        LocaleMatcher noDemotion = LocaleMatcher::Builder().
            setSupportedLocales(ARRAY_RANGE(supported)).
            setDemotionPerDesiredLocale(ULOCMATCH_DEMOTION_NONE).build(errorCode);
        Locale::RangeIterator<Locale *> desiredIter(ARRAY_RANGE(desired));
        assertEquals("no demotion",
                     "de_CH", locString(noDemotion.getBestMatch(desiredIter, errorCode)));
    }

    {
        LocaleMatcher regionDemotion = LocaleMatcher::Builder().
            setSupportedLocales(ARRAY_RANGE(supported)).
            setDemotionPerDesiredLocale(ULOCMATCH_DEMOTION_REGION).build(errorCode);
        Locale::RangeIterator<Locale *> desiredIter(ARRAY_RANGE(desired));
        assertEquals("region demotion",
                     "fr", locString(regionDemotion.getBestMatch(desiredIter, errorCode)));
    }
}

void LocaleMatcherTest::testMatch() {
    IcuTestErrorCode errorCode(*this, "testMatch");
    LocaleMatcher matcher = LocaleMatcher::Builder().build(errorCode);

    // Java test function testMatch_exact()
    Locale en_CA("en_CA");
    assertEquals("exact match", 1.0, matcher.internalMatch(en_CA, en_CA, errorCode));

    // testMatch_none
    Locale ar_MK("ar_MK");
    double match = matcher.internalMatch(ar_MK, en_CA, errorCode);
    assertTrue("mismatch: 0<=match<0.2", 0 <= match && match < 0.2);

    // testMatch_matchOnMaximized
    Locale und_TW("und_TW");
    Locale zh("zh");
    Locale zh_Hant("zh_Hant");
    double matchZh = matcher.internalMatch(und_TW, zh, errorCode);
    double matchZhHant = matcher.internalMatch(und_TW, zh_Hant, errorCode);
    assertTrue("und_TW should be closer to zh_Hant than to zh",
               matchZh < matchZhHant);
    Locale en_Hant_TW("en_Hant_TW");
    double matchEnHantTw = matcher.internalMatch(en_Hant_TW, zh_Hant, errorCode);
    assertTrue("zh_Hant should be closer to und_TW than to en_Hant_TW",
               matchEnHantTw < matchZhHant);
    assertTrue("zh should be closer to und_TW than to en_Hant_TW",
               matchEnHantTw < matchZh);
}

void LocaleMatcherTest::testResolvedLocale() {
    IcuTestErrorCode errorCode(*this, "testResolvedLocale");
    LocaleMatcher matcher = LocaleMatcher::Builder().
        addSupportedLocale("ar-EG").
        build(errorCode);
    Locale desired("ar-SA-u-nu-latn");
    LocaleMatcher::Result result = matcher.getBestMatchResult(desired, errorCode);
    assertEquals("best", "ar_EG", locString(result.getSupportedLocale()));
    Locale resolved = result.makeResolvedLocale(errorCode);
    assertEquals("ar-EG + ar-SA-u-nu-latn = ar-SA-u-nu-latn",
                 "ar-SA-u-nu-latn",
                 resolved.toLanguageTag<std::string>(errorCode).data());
}
