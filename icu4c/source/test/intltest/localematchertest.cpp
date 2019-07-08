// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// localematchertest.cpp
// created: 2019jul04 Markus W. Scherer

#include <stdio.h>  // TODO
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
