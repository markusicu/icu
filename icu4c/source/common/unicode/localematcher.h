// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License

// localematcher.h
// created: 2019may08 Markus W. Scherer

#ifndef __LOCALEMATCHER_H__
#define __LOCALEMATCHER_H__

#include "unicode/utypes.h"
#include "unicode/locid.h"
#include "unicode/stringpiece.h"
#include "unicode/uobject.h"

#ifndef U_HIDE_DRAFT_API

/**
 * Builder option for whether the language subtag or the script subtag is most important.
 *
 * @see Builder#setFavorSubtag(FavorSubtag)
 * @draft ICU 65
 */
enum ULocMatchFavorSubtag {
    /**
     * Language differences are most important, then script differences, then region differences.
     * (This is the default behavior.)
     *
     * @draft ICU 65
     */
    ULOCMATCH_FAVOR_LANGUAGE,
    /**
     * Makes script differences matter relatively more than language differences.
     *
     * @draft ICU 65
     */
    ULOCMATCH_FAVOR_SCRIPT
};
#ifndef U_IN_DOXYGEN
typedef enum ULocMatchFavorSubtag ULocMatchFavorSubtag;
#endif

/**
 * Builder option for whether all desired locales are treated equally or
 * earlier ones are preferred.
 *
 * @see Builder#setDemotionPerDesiredLocale(Demotion)
 * @draft ICU 65
 */
enum ULocMatchDemotion {
    /**
     * All desired locales are treated equally.
     *
     * @draft ICU 65
     */
    ULOCMATCH_DEMOTION_NONE,
    /**
     * Earlier desired locales are preferred.
     *
     * <p>From each desired locale to the next,
     * the distance to any supported locale is increased by an additional amount
     * which is at least as large as most region mismatches.
     * A later desired locale has to have a better match with some supported locale
     * due to more than merely having the same region subtag.
     *
     * <p>For example: <code>Supported={en, sv}  desired=[en-GB, sv]</code>
     * yields <code>Result(en-GB, en)</code> because
     * with the demotion of sv its perfect match is no better than
     * the region distance between the earlier desired locale en-GB and en=en-US.
     *
     * <p>Notes:
     * <ul>
     *   <li>In some cases, language and/or script differences can be as small as
     *       the typical region difference. (Example: sr-Latn vs. sr-Cyrl)
     *   <li>It is possible for certain region differences to be larger than usual,
     *       and larger than the demotion.
     *       (As of CLDR 35 there is no such case, but
     *        this is possible in future versions of the data.)
     * </ul>
     *
     * @draft ICU 65
     */
    ULOCMATCH_DEMOTION_REGION
};
#ifndef U_IN_DOXYGEN
typedef enum ULocMatchDemotion ULocMatchDemotion;
#endif

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
    ULOCMATCH_STORED_LOCALES  // TODO: permanent? cached?
};
#ifndef U_IN_DOXYGEN
typedef enum ULocMatchLifetime ULocMatchLifetime;
#endif

U_NAMESPACE_BEGIN

class UVector;

/**
 * Immutable class that picks the best match between a user's desired locales and
 * and application's supported locales.
 *
 * <p>Example:
 * <pre>
 * UErrorCode errorCode = U_ZERO_ERROR;
 * LocaleMatcher matcher = LocaleMatcher::Builder().setSupportedLocales("fr, en-GB, en").build(errorCode);
 * Locale *bestSupported = matcher.getBestLocale(Locale.US, errorCode);  // "en"
 * </pre>
 *
 * <p>A matcher takes into account when languages are close to one another,
 * such as Danish and Norwegian,
 * and when regional variants are close, like en-GB and en-AU as opposed to en-US.
 *
 * <p>If there are multiple supported locales with the same (language, script, region)
 * likely subtags, then the current implementation returns the first of those locales.
 * It ignores variant subtags (except for pseudolocale variants) and extensions.
 * This may change in future versions.
 *
 * <p>For example, the current implementation does not distinguish between
 * de, de-DE, de-Latn, de-1901, de-u-co-phonebk.
 *
 * <p>If you prefer one equivalent locale over another, then provide only the preferred one,
 * or place it earlier in the list of supported locales.
 *
 * <p>Otherwise, the order of supported locales may have no effect on the best-match results.
 * The current implementation compares each desired locale with supported locales
 * in the following order:
 * 1. Default locale, if supported;
 * 2. CLDR "paradigm locales" like en-GB and es-419;
 * 3. other supported locales.
 * This may change in future versions.
 *
 * TODO: no subclasses, just immutable
 * <p>All classes implementing this interface should be immutable. Often a
 * product will just need one static instance, built with the languages
 * that it supports. However, it may want multiple instances with different
 * default languages based on additional information, such as the domain.
 *
 * @draft ICU 65
 */
class U_COMMON_API LocaleMatcher final : public UMemory {
public:
    /**
     * Data for the best-matching pair of a desired and a supported locale.
     * Movable but not copyable.
     *
     * @draft ICU 65
     */
    class U_COMMON_API Result final : public UMemory {
        const Locale *desiredLocale;
        const Locale *supportedLocale;
        int32_t desiredIndex;
        int32_t supportedIndex;
        UBool desiredIsOwned;

        Result(const Locale *desired, const Locale *supported,
               int32_t desIndex, int32_t suppIndex, UBool owned) :
                desiredLocale(desired), supportedLocale(supported),
                desiredIndex(desIndex), supportedIndex(suppIndex),
                desiredIsOwned(owned) {}

    public:
        /**
         * Move constructor; might modify the source.
         * This object will have the same contents that the source object had.
         *
         * @param other Result to move contents from.
         * @draft ICU 65
         */
        Result(Result &&other) U_NOEXCEPT;

        /**
         * Destructor.
         *
         * @draft ICU 65
         */
        ~Result();

        /**
         * Move assignment; might modify the source.
         * This object will have the same contents that the source object had.
         *
         * @param other Result to move contents from.
         * @draft ICU 65
         */
        Result &operator=(Result &&other) U_NOEXCEPT;

        /**
         * Returns the best-matching desired locale.
         * nullptr if the list of desired locales is empty or if none matched well enough.
         *
         * @return the best-matching desired locale, or nullptr.
         * @draft ICU 65
         */
        inline const Locale *getDesiredLocale() const { return desiredLocale; }

        /**
         * Returns the best-matching supported locale.
         * If none matched well enough, this is the default locale.
         * The default locale is nullptr if the list of supported locales is empty and
         * no explicit default locale is set.
         *
         * @return the best-matching supported locale, or nullptr.
         * @draft ICU 65
         */
        inline const Locale *getSupportedLocale() const { return supportedLocale; }

        /**
         * Returns the index of the best-matching desired locale in the input Iterable order.
         * -1 if the list of desired locales is empty or if none matched well enough.
         *
         * @return the index of the best-matching desired locale, or -1.
         * @draft ICU 65
         */
        inline int32_t getDesiredIndex() const { return desiredIndex; }

        /**
         * Returns the index of the best-matching supported locale in the
         * constructor’s or builder’s input order (“set” Collection plus “added” locales).
         * If the matcher was built from a locale list string, then the iteration order is that
         * of a LocalePriorityList built from the same string.
         * -1 if the list of supported locales is empty or if none matched well enough.
         *
         * @return the index of the best-matching supported locale, or -1.
         * @draft ICU 65
         */
        inline int32_t getSupportedIndex() const { return supportedIndex; }

        /**
         * Takes the best-matching supported locale and adds relevant fields of the
         * best-matching desired locale, such as the -t- and -u- extensions.
         * May replace some fields of the supported locale.
         * The result is the locale that should be used for date and number formatting, collation, etc.
         *
         * <p>Example: desired=ar-SA-u-nu-latn, supported=ar-EG, service locale=ar-EG-u-nu-latn
         *
         * @return the service locale, combining the best-matching desired and supported locales.
         * @draft ICU 65
         */
        Locale makeServiceLocale() const;
    };

#if 0
    private final int thresholdDistance;
    private final int demotionPerDesiredLocale;
    private final FavorSubtag favorSubtag;

    // These are in input order.
    private final ULocale[] supportedULocales;
    private final Locale[] supportedLocales;
    // These are in preference order: 1. Default locale 2. paradigm locales 3. others.
    private final Map<LSR, Integer> supportedLsrToIndex;
    // Array versions of the supportedLsrToIndex keys and values.
    // The distance lookup loops over the supportedLsrs and returns the index of the best match.
    private final LSR[] supportedLsrs;
    private final int[] supportedIndexes;
    private final ULocale defaultULocale;
    private final Locale defaultLocale;
    private final int defaultLocaleIndex;
#endif

    /**
     * LocaleMatcher builder.
     * Movable but not copyable.
     *
     * @see LocaleMatcher#builder()
     * @draft ICU 65
     */
    class U_COMMON_API Builder final : public UMemory {
        UErrorCode errorCode = U_ZERO_ERROR;
        UVector *supportedLocales = nullptr;
        int32_t thresholdDistance = -1;
        ULocMatchDemotion demotion = ULOCMATCH_DEMOTION_REGION;
        Locale *defaultLocale = nullptr;
        ULocMatchFavorSubtag favor = ULOCMATCH_FAVOR_LANGUAGE;

    public:
        /**
         * Constructs a builder used in chaining parameters for building a LocaleMatcher.
         *
         * @return a new Builder object
         * @draft ICU 65
         */
        Builder() {}

        /**
         * Move constructor; might modify the source.
         * This builder will have the same contents that the source builder had.
         *
         * @param other Builder to move contents from.
         * @draft ICU 65
         */
        Builder(Builder &&other) U_NOEXCEPT;

        /**
         * Destructor.
         *
         * @draft ICU 65
         */
        ~Builder();

        /**
         * Move assignment; might modify the source.
         * This builder will have the same contents that the source builder had.
         *
         * @param other Builder to move contents from.
         * @draft ICU 65
         */
        Builder &operator=(Builder &&other) U_NOEXCEPT;

        /**
         * Parses the string like {@link LocalePriorityList} does and
         * sets the supported locales accordingly.
         * Clears any previously set/added supported locales first.
         *
         * @param locales the string of locales to set, to be parsed like LocalePriorityList does
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &setSupportedLocales(StringPiece locales);

        /**
         * Copies the supported locales, preserving iteration order.
         * Clears any previously set/added supported locales first.
         * Duplicates are allowed, and are not removed.
         *
         * @param locales the list of locale
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &setSupportedLocales(Locale::Iterator &locales);

        /**
         * Adds another supported locale.
         * Duplicates are allowed, and are not removed.
         *
         * @param locale another locale
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &addSupportedLocale(const Locale &locale);

        /**
         * Sets the default locale; if nullptr, or if it is not set explicitly,
         * then the first supported locale is used as the default locale.
         *
         * @param defaultLocale the default locale (will be copied)
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &setDefaultLocale(const Locale *defaultLocale);

        /**
         * If ULOCMATCH_FAVOR_SCRIPT, then the language differences are smaller than script
         * differences.
         * This is used in situations (such as maps) where
         * it is better to fall back to the same script than a similar language.
         *
         * @param subtag the subtag to favor
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &setFavorSubtag(ULocMatchFavorSubtag subtag);

        /**
         * Option for whether all desired locales are treated equally or
         * earlier ones are preferred (this is the default).
         *
         * @param demotion the demotion per desired locale to set.
         * @return this Builder object
         * @draft ICU 65
         */
        Builder &setDemotionPerDesiredLocale(ULocMatchDemotion demotion);

        /**
         * Sets the UErrorCode if an error occurred while setting parameters.
         * Preserves older error codes in the outErrorCode.
         *
         * @param outErrorCode Set to an error code if it does not contain one already
         *                  and an error occurred while setting parameters.
         *                  Otherwise unchanged.
         * @return TRUE if U_FAILURE(outErrorCode)
         * @draft ICU 65
         */
        UBool copyErrorTo(UErrorCode &outErrorCode) const;

        /**
         * Builds and returns a new locale matcher.
         * This builder can continue to be used.
         *
         * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
         *                  or else the function returns immediately. Check for U_FAILURE()
         *                  on output or use with function chaining. (See User Guide for details.)
         * @return new LocaleMatcher.
         * @draft ICU 65
         */
        LocaleMatcher build(UErrorCode &errorCode) const;
    };

    // FYI No public LocaleMatcher constructors in C++; use the Builder.

    /**
     * Returns the supported locale which best matches the desired locale.
     *
     * @param desiredLocale Typically a user's language.
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return the best-matching supported locale.
     * @stable ICU 65
     */
    Locale *getBestMatch(const Locale &desiredLocale, UErrorCode &errorCode) const;

    /**
     * Returns the supported locale which best matches one of the desired locales.
     *
     * @param desiredLocales Typically a user's languages, in order of preference (descending).
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return the best-matching supported locale.
     * @stable ICU 65
     */
    Locale *getBestMatch(Locale::Iterator &desiredLocales, UErrorCode &errorCode) const;

    /**
     * Parses the string like {@link LocalePriorityList} does and
     * returns the supported locale which best matches one of the desired locales.
     *
     * @param desiredLocaleList Typically a user's languages, in order of preference (descending),
     *          as a string which is to be parsed like LocalePriorityList does.
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return the best-matching supported locale.
     * @stable ICU 65
     */
    Locale *getBestMatch(StringPiece desiredLocaleList, UErrorCode &errorCode) const;

    /**
     * Returns the best match between the desired locale and the supported locales.
     *
     * @param desiredLocale Typically a user's language.
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return the best-matching pair of the desired and a supported locale.
     * @draft ICU 65
     */
    Result getBestMatchResult(const Locale &desiredLocale, UErrorCode &errorCode) const;

    /**
     * Returns the best match between the desired and supported locales.
     *
     * @param desiredLocales Typically a user's languages, in order of preference (descending).
     * @param lifetime Indicates whether the desired locales are temporary or stored.
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return the best-matching pair of a desired and a supported locale.
     * @draft ICU 65
     */
    Result getBestMatchResult(Locale::Iterator &desiredLocales, ULocMatchLifetime lifetime,
                              UErrorCode &errorCode) const;

    /**
     * Returns a fraction between 0 and 1, where 1 means that the languages are a
     * perfect match, and 0 means that they are completely different.
     *
     * <p>This is mostly an implementation detail, and the precise values may change over time.
     * The implementation may use either the maximized forms or the others ones, or both.
     * The implementation may or may not rely on the forms to be consistent with each other.
     *
     * <p>Callers should construct and use a matcher rather than match pairs of locales directly.
     *
     * @param desired Desired locale.
     * @param supported Supported locale.
     * @param errorCode ICU error code. Its input value must pass the U_SUCCESS() test,
     *                  or else the function returns immediately. Check for U_FAILURE()
     *                  on output or use with function chaining. (See User Guide for details.)
     * @return value between 0 and 1, inclusive.
     * @internal (has a known user)
     */
    double internalMatch(const Locale &desired, const Locale &supported, UErrorCode &errorCode) const;
};

U_NAMESPACE_END

#endif  // U_HIDE_DRAFT_API
#endif  // __LOCALEMATCHER_H__