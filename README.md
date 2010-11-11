# Fuga

This is the (main) interpreter for the programming language, Fuga. The
interpreter is also known as Fuga. To learn more about the language,
visit www.fugal.org.

Fuga (the interpreter) has three goals:

* *Expressivity*: Code should flow naturally. The language should not
constrain the programmer. This means we can't compromise expressiveness for
ease of implementation and whatnot.

* *Embeddability*: Other projects should find it easy embed Fuga into their
applications, and to use Fuga as a scripting language. This means that the
interpreter must have a simple and well defined C API, and that it should be
fairly portable.

* *Extensibility*: It should be easy for programmers to add their own
extensions to the language, both from inside (i.e. in Fuga code) and outside
(i.e. using the C API).

In spite of the embeddability requirement, this project is a standalone
interpreter, whose core can be taken and used as in an embedded way.

## Directory Structure

Fuga has the following top-level directory structure:

* `src/` - source files
    * `src/fuga/` - the embeddable interpreter's source/header files
* `bin/` - binary files (object files)
* `test/` - various test executables
* `tools/` - tools for building and testing Fuga

## Building

To build Fuga, use `tools/make`. For example,

    $ tools/make fuga/gc
    
Will build `fuga/gc.c` and all of its dependencies. Like the traditional 
`make`, it only builds files that have changed since the last time they 
were built (or if the `.o`) file is missing from the `bin` folder.

Once the project is further along `tools/make` should build the interpreter
by default, and `tools/make --install` will install it once it's built.

## Style

In order to keep the C API simple, consistency is a must. Here are some rules
that were designed to enhance consistency.

### Naming Conventions

Global identifiers must begin with the prefix `Fuga` or `FUGA_`. Global
identifiers that are meant to be used only inside the Fuga source code (and
not in the surrounding application) use `_Fuga` or `_FUGA_` instead.

Type names are in `CamelCase`, with the first letter capitalized, and the
first letter of subsequent words also capitalized. For example, the following
are all types: `Fuga`, `FugaInt`, `FugaGC`, `_FugaGCHeader`.

Functions almost always pertain mainly to one type (think method in an OO
language). Function names begin with that type name, followed by an 
underscore, followed by a name. For example, the following are all function
names: `Fuga_new`, `FugaInt_add`, `FugaGC_alloc`.

In general, the names of functions after the underscore use camelCase, with
the first letter lowercase and the first letter of each subsequent word
capitalized. For example: `Fuga_fooBarBaz`.

Macros and #defines begin with the `FUGA_` or `_FUGA_` prefixes, and are
entirely uppercase, using underscores to separate words. For example: 
`FUGA_INIT_CORE`, `_FUGA_GC_HEADER`.

### Formatting

Don't use tabs -- use spaces. Four spaces, to be exact.

### Documentation

Document, document! Heavily document! Documentation is to precede the
code in question, and should take the form:

    /**
    *** documentation
    *** goes
    *** here
    **/

Documentation is to be in Markdown format, where headers are prefixed
with `#` or `##`, small code snippets are enclosed in backquotes (\`),
and large code snippets are set off and indented, as such:

    /**
    *** Here's some sample code:
    ***
    ***     int fact(x) {
    ***         if (x < 1) return 1;
    ***         return x * fact(x-1);
    ***     }
    ***
    *** Isn't it pretty?
    **/

For a primer on Markdown syntax search for "Markdown" using your favorite
search engine.

Generally, one should document the interface to a module in its header
file, and document the implementation in the source file(s). This way,
a developer who wishes to use a module need only look in one place, and a
developer who wishes to modify a module can quickly get to grips with the
existing code, and can tell what can and can't be modified without
repercussions.

Here's an example of documentation of a function (note that this isn't a real 
function in Fuga, so the function name doesn't follow any convention).

In the header file:

    /** ### max
    ***
    *** `max` returns the greater of its two arguments.
    ***
    *** - Parameters:
    ***     - `int a`: the first argument
    ***     - `int b`: the second argument
    *** - Returns: `a` or `b`, whichever argument is greater.
    *** - See also: `min`
    **/
    int max(int a, int b)

In the source file:

    /** ### max
    ***
    *** In `max` we compare `a` with `b` and return whichever is greater.
    *** If they are both the same, we return `a`, in order to preserve
    *** order (this would be more useful if `>=` were overloaded somehow).
    **/
    int max(int a, int b) {
        if (a >= b)
            return a;
        return b;
    }

Granted, this source code documentation might be a bit overkill for this
particular function, but it usually doesn't hurt to elaborate a bit on the
techniques used and decisions made when implementing a function.

The greatest enemy of documentation is laziness. Hopefully I can develop some
sort of tool to combat this laziness. Meanwhile, one needs to be considerate
and update one's documentation alongside the code, not after the fact (or
worse, never).

In the future, there will be a tool to turn all of this documentation into
Markdown / HTML. That's why the documentation has to be in Markdown format:
because Markdown makes it easy to convert it into HTML.

## Testing

`fuga/test.h` contains a useful testing framework, when paired with
`tools/test`. Because of the generality of this package, and because it
only exports a handful of symbols, and for historical reasons, identifiers
aren't prefixed with `FugaTest_` or `FUGA_TEST_`.

For more information about testing, look in `src/fuga/test.h`.

### Assertions

Like sqlite, we use

    ALWAYS(x)

and

    NEVER(x)

to build in assertions into the code. On release code (or, performance
testing code) these are removed.

### Test cases

Similar to sqlite (where this macro is known as `testcase`), we use

    TESTCASE(x)

To specify a test case within the code. Thus, when we are testing for
test coverage, these macros help to identify which cases are missing.
Our goal is to test all branches, so we should use these before branches,
to test every possible condition.

### Unit tests

To actually test code, we must develop test cases. Tests are organized
into suites. The general syntax / schematic for a test suite is:

    TESTSUITE(suite_name) {
        // set things up
        int a = 10;
        int b = a+10;
        int c = b/2;
        int *x = malloc(sizeof *x);
        // run tests
        TEST(a == c);
        // perform clean up
        free(x);
    }

Test suites are to be located within the implementation file. To declare
a suite in a header file, use

    TESTSUITE(suite_name);

Most suites need not be declared in header files, however.

To call a test suite from another suite, use:

    TESTSUITE_CALL(suite_name);

To call a test suite from outside, use:

    TESTSUITE_RUN(suite_name);

To run a test suite, use the hand tools/test:

    $ tools/test suite_name

You can run all test suites in a file or directory by passing those
to tools/test instead, or you can run all test suites.

    $ tools/test filename
    $ tools/test directory
    $ tools/test --all

At this point, don't pass in the .c if you want to test a file! That's 
added automatically. In the future, the .c will be allowed, but right now
it isn't.
