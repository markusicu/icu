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

class LocaleMatcherTest : public IntlTest {
public:
    LocaleMatcherTest() {}

    void runIndexedTest(int32_t index, UBool exec, const char *&name, char *par=NULL);

    void testBasics();
};

extern IntlTest *createLocaleMatcherTest() {
    return new LocaleMatcherTest();
}

void LocaleMatcherTest::runIndexedTest(int32_t index, UBool exec, const char *&name, char * /*par*/) {
    if(exec) {
        logln("TestSuite LocaleMatcherTest: ");
    }
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO(testBasics);
    TESTCASE_AUTO_END;
}

void LocaleMatcherTest::testBasics() {
    IcuTestErrorCode errorCode(*this, "testBasics");
    Locale locales[] = { "de-LU", "en-GB", "de-AT", "fr" };
    Locale::RangeIterator<Locale *> iter(locales, locales + UPRV_LENGTHOF(locales));
#if 0
    std::vector<Locale> locales{ "de-LU", "de-AT" };
    Locale::RangeIterator<std::vector<Locale>::iterator> iter(locales.begin(), locales.end());
#endif
    Locale def("fr");
    LocaleMatcher matcher = LocaleMatcher::Builder().setSupportedLocales(iter).setDefaultLocale(&def).build(errorCode);
#if 0
    Locale *pointers[] = { locales, locales + 1 };
    LocaleMatcher matcher = LocaleMatcher::Builder().
        setSupportedLocalesViaConverter(pointers, pointers + UPRV_LENGTHOF(pointers),
                                        [](const Locale *p) { return *p; }).
        build(errorCode);
#endif
#if 0
    LocaleMatcher::Builder builder;
    for (const Locale &locale : locales) {
        builder.addSupportedLocale(locale);
    }
    LocaleMatcher matcher = builder.build(errorCode);
#endif
    Locale desired("ru");
    const Locale *best = matcher.getBestMatch(desired, errorCode);
    printf("best match: '%s'\n", best != nullptr ? best->getName() : "null");
}
