## Directory Structure

Fuga has the following top-level directory structure:

* `src/` - source files
    * `src/fuga/` - the embeddable interpreter's source/header files
* `bin/` - binary files (object files)
* `test/` - various test executables
* `tools/` - tools for building and testing Fuga

## Building

To build and test Fuga, just use `make`:

    $ make

You can rapidly make changes to code and then try them out by using
`make try`, which opens up a REPL.

To build specific Fuga modules, use `tools/make`. For example,

    $ tools/make fuga/gc

Will build `fuga/gc.c` and all of its dependencies. Like the traditional 
`make`, it only builds files that have changed since the last time they 
were built (or if the `.o`) file is missing from the `bin` folder.

## Style

### Naming Conventions

Global identifiers must begin with the prefix `Fuga` or `FUGA_`. Global
identifiers that are meant to be used only inside the Fuga source code (and
not in the surrounding application) use `_Fuga` or `_FUGA_` instead.

Type names are in `CamelCase`, with the first letter capitalized, and the
first letter of subsequent words also capitalized. For example, the
following are all types: `Fuga`, `FugaInt`, `FugaGC`, `_FugaGCHeader`.

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

Don't use tabs. Always indent using four spaces.

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

    /**
    *** ### max
    ***
    *** Return the greater of two arguments.
    ***
    *** - Params:
    ***     - `int a`: the first argument
    ***     - `int b`: the second argument
    *** - Return: `a` or `b`, whichever argument is greater.
    *** - See also: `min`
    **/
    int max(int a, int b)

In the source file:

    /**
    *** ### max
    ***
    *** We compare `a` with `b` and return whichever is greater.
    **/
    int max(int a, int b) {
        if (a >= b)
            return a;
        return b;
    }

Granted, this source code documentation might be a bit overkill for this
particular function, but it usually doesn't hurt to elaborate a bit on the
techniques used and decisions made when implementing a function.

In the future, there will be a tool to turn all of this documentation
into Markdown / HTML. That's why the documentation has to be in Markdown
format.

## Testing

`fuga/test.h` contains a useful testing framework, when paired with
`tools/test`. Because of the generality of this package, and because it
only exports a handful of symbols, and for historical reasons, identifiers
aren't prefixed with `FugaTest_` or `FUGA_TEST_`.

For more information about testing, look in `src/fuga/test.h`.

### Assertions

We use

    ALWAYS(x)

and

    NEVER(x)

to build in assertions into the code. On release code (or, performance
testing code) these assertions are removed.

### Unit tests

To actually test code, we must develop unit tests. Tests are organized
into suites. The general syntax / schematic for a test suite is:

    TESTS(suite_name) {
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

You can run all test suites in a file or directory in src by passing those
to tools/test, or you can run all test suites. For example,

    $ tools/test fuga/fuga

will run all tests in `src/fuga/fuga.c`,

    $ tools/test fuga

will run all tests in `src/fuga/*.c`,

    $ tools/test --all

will run all tests.

