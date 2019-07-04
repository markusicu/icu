// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// localematchertest.cpp
// created: 2019jul04 Markus W. Scherer

#include "unicode/utypes.h"
#include "unicode/localematcher.h"
#include "unicode/locid.h"
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
    LocaleMatcher matcher = LocaleMatcher::Builder().build(errorCode);
}
