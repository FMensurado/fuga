# Fuga

Specification Draft
16 February 2011

## 1. Table of Contents

1. Table of Contents
2. Abstract
3. Overview
4. Syntax
5. Semantics
6. The Prelude

## 2. Abstract

We define the programming language, Fuga, which is object-oriented like
Self, and homoiconic like Lisp. In this specification, we lay out the
syntax and the semantics of the language, as well as 

## 3. Overview

Fuga is a pure object-oriented programming language. That is, all values
in Fuga are objects. All objects (and therefore all values) share the 
following properties:

- All objects have an identity. 
- All objects have ordered slots.
- All objects (except `Object`) have a prototype -- another object.
- All objects respond to messages.

An object's identity is used to determine whether two objects are the
same. If object A and object B have the same identity, we say that 
A is B.

All slots have a pointer to another object -- we call this object the
slot's value. A slot may also have a name. If a slot has a name, the
name must be unique among that object's slots, and the name must be a
symbol primitive (defined below). Slots may also have documentation --
unlike most languages that allow for a first-class documentation, the
documentation in Fuga is associated with the slot, not with the slot's
value.

An object's prototype is simply another object. Intuitively, when an
object doesn't recognize a message, it resends the message to its
prototype. If object A's prototype is object B, we say that A inherits
from B.

Some objects also hold a third kind of data -- primitive data. Primitive
data allows us to define integer and string and symbol objects, and more,
by bringing in data outside of the object system. Objects with primitive
data are considered "primitive", and are called "primitives".

Fuga has five built-in types of primitives:

- Integer primitives contain integer data, which corresponds to the C
type `long`. All integer primitives inherit from `Int`.

- String primitives contain string data -- sequences of non-zero
characters. All string data is encoded in UTF-8. All string primitives
inherit from `String`.

- Symbol primitives also contain string data, although there are rules
regarding what is or isn't a valid symbol (see Syntax below). Symbol
primitives inherit from `Symbol`.

- Method primitives contain a function pointer. We use method primitives
to supply functionality from outside of Fuga. For example, `Int` methods
such as `+` (adding) or `-` (subtracting) are implemented as method
primitives. Method primitives inherit from `Method`.

- Message primitives contain a pointer to a name. The name can be either
a symbol primitive, or an integer primitive. Messages are used to invoke
methods or access slot values. Message primitives inherit from `Msg`.

## 4. Syntax

How do we represent objects and primitives? This is going to be an
intuitive description at Fuga's syntax. For a formal specification
of the syntax, go to Appendix A, which includes a formal grammar.

Non-primitive objects are represented as a sequence of slots enclosed
in parentheses:

    ( slots )

An empty object is simply `()`. Objects represented in this way inherit
directly from `Object`. You can separate slots by using commas or
newlines. Exraneous commas or newlines are simply ignored.

    (slot1, slot2
    slot3,,slot4,,,
    ,,)

Slots can contain any expression. Let's look at how to represent
various primitives before we move on to expressions.

Integers are any sequence of digits 0 through 9. These are all integers:

    0
    2
    123
    007

Strings are enclosed in double quotes. You can use backslash to escape
quotes, or for special characters (such as a newline `\n`, tab `\t`,
or carriage return `\r`). These are all strings:

    ""
    "Hello, world!"
    "Hello!\n"
    "first name\tlast name\n"
    "He said, \"hello world,\" and went on his merry way."

Symbols begin with a colon, and generally are made up of letters and 
digits, underscores, question marks, and exclamation points.
Alternatively, one can escape an operator using a backslash. A symbol
cannot be made up exclusively of digits. These are all valid symbols:

    :do
    :1st
    :hello_world
    :hello?!
    :\+
    :\=

A message has two components -- the name component, and the argument
component. The name component is either a symbol without the colon, or
a number:

    do
    1st
    hello_world
    hello?!
    \+
    10

The argument component is an object. If the argument is empty, it can
be ommited altogether. Here are some example messages:

    hello_world("he said")
    do(hello, world)
    if(a == 10, 

An expression is a root followed by zero or more messages. The root can
be any of the primitives with syntax (integers, strings, symbols, and
messages), and it can be an object. The following are expressions:

    10
    "Hello, world!" print
    10 \+(20)
    

