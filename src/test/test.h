#ifndef TEST_H
#define TEST_H

/*
** This is included in order to use the assert() macro. In the future, we
** might consider rolling our own assert().
*/
#include <assert.h>

/*
** The macros ALWAYS and NEVER perform assert operations. They are meant
** to be used whenever something is known to be true or false. In release
** code, these are stripped out, for performance. In coverage code, the
** assertions are removed in order to avoid unnecessary branches.
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
#ifdef TEST_COVERAGE
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
** The TESTCASE macro is used to specify test cases within the code.
** Simply add a condition that should be tested for. These are to
** be used to check for test coverage.
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
#ifdef TEST_COVERAGE
# define TESTCASE(X) // TODO: figure this out
#else
# define TESTCASE(x)
#endif




