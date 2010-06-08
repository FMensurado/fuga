#ifndef TEST_H
#define TEST_H

/*
** This file can be invoked/compiled with the following pre-processor
** options pre-defined:
**
** * none - no test suites, no assertions
** * `DEBUGGING` - assertions will be compiled.
** * `TESTING` - test suites will be compiled and assertions will be
** compiled. DEBUGGING is turned on automatically.
** * `TESTING_COVERAGE` - test suites will be compiled, but test cases are
** only being tested for coverage, not for success.
**
** Note that it is not possible to test both the code (`TESTING`) and the
** tests (`TESTING_COVERAGE`) at the same time. Attempting to do so will
** raise an error.
*/
#ifdef TESTING
# ifdef TESTING_COVERAGE
#  error "Cannot test code (`TESTING`) and tests (`TESTING_COVERAGE`)"\
         ## "simultaneously."
# endif

# ifndef DEBUGGING
#  define DEBUGGING
# endif
#endif


/*
** This is included in order to use the assert() macro. In the future, we
** might consider rolling our own assert().
*/
#include <assert.h>

/*
** The macros `ALWAYS` and `NEVER` perform `assert` operations. They are
** meant to be used whenever something is known to be true or false. In
** release builds, these are stripped out for performance. In test
** coverage builds, the assertions are removed in order to avoid
** unreachable branches.
**
** Example:
**
**     int max(int *xs, size_t len) {
**         NEVER(xs == NULL);
**         ALWAYS(len > 0);
**         int curMax = *(xs++);
**         while (--len) {
**             if (*xs > curMax) curMax = *xs;
**             xs++;
**         }
**         ALWAYS(len == 0);
**         return curMax;
**     }
*/
#ifdef TESTING_COVERAGE
# define ALWAYS(x) (1)
# define NEVER(x)  (0)
#elif defined(DEBUGGING)
# define ALWAYS(x) assert(x)
# define NEVER(x)  assert(!(x))
#else
# define ALWAYS(x)
# define NEVER(x)
#endif

/*
** The `TESTCASE` macro is used to specify test cases within the code.
** Simply add a condition that should be tested for. These are to be used
** to check for test coverage.
**
** Example:
**
**     int max(int *xs, size_t len) {
**         int curMax = *(xs++);
**         TESTCASE(len == 1);
**         TESTCASE(len == 2);
**         TESTCASE(len > 1024);
**         while(--len) {
**             if (*xs > curMax) curMax = *xs;
**             xs++;
**         }
**         return curMax;
**     }
*/
#ifdef TESTING_COVERAGE
# define TESTCASE(X) // TODO: figure this out
#else
# define TESTCASE(x)
#endif

/*
** ## `TESTSUITE(suite_name, suite_body)` 
**
** Use `TESTSUITE` to create a test suite / a wrapper around unit tests.
** Ideally there should be a 1-to-1 correspondence between tests suites
** and functions/modules (see the top-level README file, for information
** on modules). If more than one test suite is needed (in order to
** keep the test suites readable), they should be numbered, and there
** should be one overarching test suite that calls all the others.
**
** To call a suite from outside, use
**
**     TESTSUITE_RUN(suite_name);
**
** To call a suite from within a suite, use
**
**     TESTSUITE_CALL(suite_name);
**
** Parameters:
**
** * `suite_name` is modified before it becomes an actual identifier,
** so `suite_name` can and should be the same as the function or module
** it purports to test.
**
** * `suite_body` is a series of statements. Use the `TEST` family of
** macros to add assertions / unit tests (see below for the documentation
** for `TEST`). The general structure for a suite is to set up values for
** testing beforehand, then make assertions, and then clean up.
**
** Example:
**
**     TESTSUITE(foo_bar,
**         // set up
**         int x = foo_bar();
**         int y = foo_bar();
**         // run tests
**         TEST(x == 10, "The first call to foo_bar should equal to 10");
**         TEST2(x != y, "Consecutive calls to foo_bar should be different.\n"
**                       "    (1st call: %d, 2nd call: %d)", x, y);
**         // clean up
**         // (nothing to do)
**     )
*/

#define TESTSUITE(suite_name, suite_body)                     \
    size_t suite_name##_test(const char* _suite_name) {       \
        size_t _tests_failed = 0;                             \
        suite_body                                            \
        if (_tests_failed) {                                  \
            printf("[%s] Failed %lu tests.\n",                \
                _suite_name,                                  \
                _tests_failed);                               \
        }                                                     \
        return _tests_failed;                                 \
    }

#define TESTSUITE_CALL(suite_name)                            \
    _tests_failed += suite_name##_test(#suite_name)

#define TESTSUITE_RUN(suite_name)                             \
    suite_name##_test(#suite_name)


/*
** ## `TEST`, `TEST1`, `TEST2`, `TEST3`, `TEST4`
** 
** The `TEST` family of macros checks an assertion and prints out a
** message if it is false. The message should explain what the expected
** outcome was and/or justify why the test exists.
**
** There are several `TEST` macros, because we want to be able to print
** the values that caused our test to fail. To do this, the `message` is
** passed to `printf`, along with any other arguments. So, the number in
** front of `TEST` specifies the number of additional arguments to
** `printf`.
**
**     TEST(assertion, message)
**     TEST1(assertion, message_format, arg1)
**     TEST2(assertion, message_format, arg1, arg2)
**     etc
**
** When we're testing for test coverage, we only want to run the
** assertions, but not actually branch on the assertion. This is in order
** to activate any branches that we need, without creating unreachable
** branches in the process.
**
** `TEST` must only be called from within a `TESTSUITE`, or it won't
** compile. (This is on purpose.) For an example of `TEST` usage, look at
** the `TESTSUITE` documentation above.
*/

#ifdef TESTING_COVERAGE

# define TEST(assertion, message)                  (assertion)
# define TEST1(assertion, message, a1)             (assertion)
# define TEST2(assertion, message, a1, a2)         (assertion)
# define TEST3(assertion, message, a1, a2, a3)     (assertion)
# define TEST4(assertion, message, a1, a2, a3, a4) (assertion)

#elif defined(TESTING)

# define TEST(assertion, message) \
    if (!(assertion)) { _tests_failed++; puts(message); }

# define TEST1(assertion, message, arg1)  \
    if (!(assertion)) { _tests_failed++; \
        printf(message, arg1);           \
        puts("");                        \
    }

# define TEST2(assertion, message, arg1, arg2) \
    if (!(assertion)) { _tests_failed++;      \
        printf(message, arg1, arg2);          \
        puts("");                             \
    }

# define TEST3(assertion, message, arg1, arg2, arg3) \
    if (!(assertion)) { _tests_failed++;            \
        printf(message, arg1, arg2, arg3);          \
        puts("");                                   \
    }

# define TEST4(assertion, message, arg1, arg2, arg3, arg4) \
    if (!(assertion)) { _tests_failed++;                  \
        printf(message, arg1, arg2, arg3, arg4);          \
        puts("");                                         \
    }

#endif

#endif // #ifndef TEST_H
