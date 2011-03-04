# Fuga

Specification Draft  
Started: 16 February 2011  
Updated: 24 February 2011

## 1. Table of Contents

1. Table of Contents

2. Overview

    1. All values are objects.
    2. Prototypes rather than classes.
    3. Strong, Dynamic Typing
    4. Homoiconicity
    5. Optional Lazy Evaluation
    6. First-class Thunks

3. Syntax

    1. Full Grammar
    2. Operator Precedence
    3. Whitespace and Comments

4. Semantics

5. The Prelude

## 2.  Overview

This document defines Fuga, a pure object-oriented programming language.
Fuga's defining features are that all values are objects, prototyping
replaces the traditional class-based object system, typing is strong
and dynamic, and that Fuga is homoiconic. Another aspect of Fuga is that
it supports both strict and lazy evaluation, and it support first-class
thunks.

Let us look at what is meant by each of the previous claims, before
digging into specifics of the language's syntax and semantics.

### 2.1. All values are objects.

This includes booleans, integers, strings, symbols, messages,
expressions, methods, thunks, etc.

All objects contain an ordered sequence of slots, and all objects
respond to messages. A slot is a value holder. A slot may also hold
a name symbol, and it may also hold a documentation string. In
general, an object responds to a message by looking for a slot with
the corresponding name. For example, if we define an object A with
a slot with name "foo", A will respond to the message "foo" by finding
that slot and then performing the appropriate action.

We make a distinction between primitive and non-primitive objects. A 
primitive object is an object which contains raw, non-object data. For
example, an integer primitive contains raw integer data. Primitives
are still objects and work exactly like objects in every other way (to
the point that primitives have slots, like all other objects).

### 2.2. Prototypes rather than classes.

In class-based systems, methods are part of class definitions, and
objects contain only data. In Fuga, objects contain methods AND data,
and there are no classes.

Instead, when creating an object, we can specify its prototype. The
prototype of an object is itself an object. Whenever we send a message
to an object, and the object doesn't contain a corresponding slot, the
message is sent to the object's prototype instead. This way, we can
generalize behavior, and have an organized object system like in a
traditional class-based language.

### 2.3. Strong, Dynamic Typing.

Fuga is dynamically typed -- Fuga does not check types ahead of time.
Rather, Fuga trusts you to make good decisions, and this gives you more
flexibility in how to structure your code.

Fuga has a built in exception system, and whenever you call a built-in
function with the wrong type, Fuga will raise an exception. This is 
why we say Fuga has strong typing. If Fuga was weakly typed, it would
try to coerce types as needed. Strong typing makes it easier to find bugs
in dynamically-typed code.

### 2.4. Homoiconicity

Fuga is a pure object-oriented programming language. Naturally, Fuga can
be used to manipulate objects. In order to support high levels of
metaprogramming, Fuga exposes the code as data.

Another aspect of homoiconicity is that data and code are expressed in
the same way, on a syntactic level.

### 2.5. Optional Lazy Evaluation

Sometimes, it is useful to hold off on computing something until its
result is needed. This is what lazy evaluation gives us. In Fuga, 
expressions are evaluated strictly by default, but Fuga supports
lazy evaluations as needed. A thunk (a lazy computation that has yet to
be evaluated) is evaluated whenever a message is sent to it, or
whenever you try to access or modify its slot information.

### 2.6. First-class thunks.

Fuga supports call-by-thunk and first-class thunks. This means that
methods decide which arguments are evaluated, when, and how often, as
well as having access to the caller's environment. This brings a great
deal of expressivity to the language, as even "if"-like constructs can
be defined in the language itself. For example,

    true   then = method((~body), body eval)
    false  then = method((~body), nil)
    nil    else = method((~body), body eval)
    Object else = method((~body), self)

No you can use `[cond] then(expr) else(expr)`, as an `if`-like
expression:

    factorial = method((n),
        [n == 0] then(1) else(n * factorial(n-1))
    )
    

## 3. Syntax

When discussing Fuga syntax, it helps to present the grammar without 
syntactic sugar, and the grammar with syntactic sugar separately.
Here is the grammar without syntactic sugar:

    <block> ::= <slot> <separator> <block>
              | <separator> <block>
              |
    <slot>  ::= <doc> <separator> <slot>
              | <expr> <doc>
              | <expr>
    <expr>  ::= <root> <msgs>
    <root>  ::= <integer>
              | <string>
              | <symbol>
              | <object>
              | <msg>
    <msgs>  ::= <msg> <msgs>
              |
    <msg>   ::= <name> <object>
              | <name>
    <name>  ::= <symbol-name>
              | <integer>
    <object> ::= "(" <block> ")"
    <separator> ::= ","
                  | "\n"
    <symbol> ::= ":" <symbol-name>
    <symbol-name> ::= <actual-name>
                    | "\" <operator>

The important part is that an object is a collection of slots, slots
contain an expression, and all expressions are a root followed by any
number of messages. Below, I've included regular expressions to describe
the missing parts.

    <doc>:          :: ?[^\n]*
    <integer>:      (0d)?[0-9]+|0x[0-9a-fA-F]+|0o[0-7]+|0b[01]+
    <string>:       "([^\\"]|\\[\\"ntr])*"
    <actual-name>:  [a-zA-Z0-9_?!]+
    <operator>:     [`~!@#$%^&*\-+=|\\:;"'<,>.?/]+

There's some overlap in these token definitions. For example, the
regular language that describes integers is a subset of the regular
language that describes actual names. Likewise, a single colon (:)
must be treated as a separate token, not as an operator. To resolve
this, go with whichever regular expression happens to match first.

There is one further complication. An operator that is immediately
followed by a left parenthesis is treated as an actual-name, not
as an operator. So,

    a +(b)

is legal according to the grammar above. But

    a + (b)

is illegal (there is no way to integrate operators unless they are
escaped with a backslash). In the full grammar below, this is an
actual expression, but it means something very different. Than
`a +(b)`.

A message without an object is simply treated as a message with
an empty object. So, `a()` and `a` are the same. The object is
called the message's argument.

Therefore, one thing to notice is that argument lists are objects,
unlike in most languages. `(10, 20, 30)` is an object. `foo(10, 20, 30)`
is a message with name `foo` and argument `(10, 20, 30)`.

### 3.1 Operators

Unlike the grammar above, Fuga allows for infix and prefix operator
expressions. All that needs to be modified in the grammar above is

    <expr> ::= <expr-part> <operator> <expr>
             | <expr-part>
    <expr-part> ::= <root> <msg>
    <root> ::= <operator> <root>
             | "[" <expr> "]"
             | <integer>
             | <string>
             | <symbol>
             | <object>
             | <msg>

Prefix and infix operator expressions are converted into expressions
as per the simplified grammar. Prefix operators are become messages
sent to their operand:

    +a

becomes

    a \+

Where \+ is the "escaped" version of the operator, turning it into a
simple message.

Infix operators become messages to their left operand:

    a + b

becomes

    a \+(b)

The exception to this rule is `=`: `a = b` becomes `\=(a, b)`.

### 3.2 Operator Precedence

When there are multiple infix operators in an expression, their
precedence becomes relevant. An operator with a higher precedence
binds more tightly than an operator with a lower precedence. For
example, `*` has a higher precedence than `+`.

Built in operator precedence:

    1:   =
    500: < > == <= >=
    600: ++
    1500: + -
    1510: * % // /
    1520: **

All other operators (including user-defined operators) have a default
precedence of 1000.

To override the rules of precedence, you must group your expressions
with square brackets:

    10 + 20 * 30                # becomes 10 \+(20 \*(30))
    [10 + 20] * 30              # becomes 10 \+(20) \*(30)

Finally, all operators are left associative in Fuga. This means that

    10 + 20 + 30

becomes

    10 \+(20) \+(30)

rather than

    10 \+(20 \+(30))

Only left associativity is supported in order to keep things simple,
to lessen the cognitive burden of using custom operators.

### 3.3 Methods

There is one further syntactic sugar to add to the grammar.

    <expr-part> ::= <root> <msgs> <method>
                  | <root> <msgs>
                  | <method>
    <method> ::= "{" <block> "}"

FIXME: Explain how to desugar methods.

To desugar a method, we must consider the four cases where methods
are present.

Case 0: The method is anonymous has no arguments. For example,

    { print("Hello!") }

is an anonymous method that prints the line "Hello!". Such methods
are converted into a `method` message, with the first argument
being an empty object, and the rest being the body of the method. The
above example would become

    method((), print("Hello!"))

Case 1: The method is anonymous and has arguments. For example,

    (a, b) { a*a + b*b }

This anonymous method calculates the sum of squares. This desugars
like the previous kind of method, but the arguments are used as the
first argument to `method`. This would be converted into,

    method((a, b), a*a + b*b)

Case 2: The method is not anonymous, and has no arguments. The expr-part
before the method body becomes the left-hand side for `=`.

    foo { 20 }

This is converted to

    \=(foo, method((), 20))

Case 3: The method is not anonymous, and has arguments. The expr-part
before the method body becomes the left-hand side for `=`, except for
the arguments to the last message, which become the method's arguments.

    foo(a, b) { a + b }

Is converted to

    \=(foo, method((a, b), 20))


### 3.4 Full grammar

The full grammar, with all of the syntactic

### 3.4 Whitespace and Comments

Most newlines are significant; other forms of whitespace are ignored.
Newlines that follow an operator or "[" are ignored. Newlines that
precede "]" are also ignored. If the newline follows a backslash, both
the newline and the backslash are ignored.

Fuga supports only single-line comments. Comments begin with "#" and
go to the end of the line. Comments are ignored, but the newline at
the end of a comment isn't, unles

## 4. Semantics

### 4.1. Object Behaviors

As mentioned previously, all values are objects. We only gave an informal
overview of what it means to be an object, so let us define what is meant
by the word "object." We shall define "object" by giving a list of
behaviors shared by all objects.

- **is**: All objects have an identity. You can test whether two objects
the same identity.

- **has slot**: All objects have slots. You can test whether an object
contains a slot with a given name, or a given index. You can also test
whether a given slot has a name, or has documentation.

- **get slot**: You can retrieve the value of a slot with a given name,
or a given index. You can also retrieve the slot's name, or the slot's
documentation.

- **set slot**: You can associate a name or an index with a given value,
creating or updating a slot in the process. Indexed slots are not sparse,
so you must define slot 0, 1, and 2, before you can define slot 3.

- **proto**: Most objects have prototypes. You can retrieve that
prototype.

- **num slots**: You can retrieve the number of slots an object has.

- **primitive**: Primitives contain primitive data. Primitives also
identify their primitive type, in order to distinguish primitive
integers from primitive strings, primitive symbols, etc.

- **call**: Methods and method primitives can be called, with a given
argument and a given "self". Non-methods can also be called, but they
have no effect, other than checking to make sure that no arguments were
passed.

These are an object's basic behaviors. There are common behaviors
derived from these basic behavios:

- **is a**: Test whether an object inherits from another. This is a
combination of **is** and **proto**.

- **has**: You can search both the object's slots and the object's
prototypes for a slot with the given name. This is a combination of
**has slot** and **proto**.

- **get**: Again, you can search an object's prototype if a given
slot is not available to the current object. This is a combination
of **get slot** and **proto**.

- **append**: You can add an indexed slot after all other slots. This
is a combination of **set slot** and **num slots**.

- **send**: Send a message. This involves getting the associated slot,
and calling the associated value. This is a combination of **get** and
**call**.

With send, we can define a myriad of other behaviors. The most important
behavior that derives from send is **eval**, which explains how the
program structure is converted into actual values. We define it in
section 4.3. In the next section, we define some of Fuga's built-in
objects, and how they relate to the built-in primitives.

### 4.2 Built-in Objects

- `Object` is the root object. `Object` has no prototype, but all other
objects must inherit from `Object`.

- `Number` is a base object for number types, such as integers,
floating point numbers, rational numbers, etc. `Number`'s prototype is
`Object`.

- `Int` is the base object for integer primitives. `Int`'s prototype is
`Number`.

- `String` is the base object for string primitives. `String`'s prototype
is `Object`.

- `Symbol` is the base object for symbol primitives. `Symbol`'s prototype
is `Object`.

- `Msg` is the base object for message primitives. `Msg`'s prototype is
`Object`.

- `Method` is the base object for method primitives. `Method`'s prototype
is `Method`

- `Expr` is the base object for expressions. An expression is a sequence
of messages. The expression object's slots represent this sequence.

- `Bool` is the base object for booleans. `Bool`'s prototype is `Object`.

- `true` is the positive boolean value. `true`'s prototype is `Bool`.

- `false` is the negative boolean value. `false`'s prototype is `Bool`.

- `Prelude` is the base object for new modules. Section 5 explains the 
module system in detail, including built-in modules.

### 4.3 Evaluation

After parsing, our code consists of integer primitives, string
primitives, symbol primitives, message primitives, expressions, and
vanilla objects (that is, objects that inherit directly from `Object`).
None of these objects will contain named slots, so we can define behavior
on code as methods of `Int`, `String`, `Symbol`, `Msg`, `Expr`, and
`Object`.

Thus we will define evaluation as methods of those objects. Evaluation
has three inputs: the object to be evaluated, the lexical scope in which
to evaluate it, and the current receiver. In most cases, the current
receiver is the lexical scope.

To begin, integer, string, and symbol primitives evaluate to themselves,
regardless of the environment.

    Int    eval(scope, recv) { self }
    String eval(scope, recv) { self }
    Symbol eval(scope, recv) { self }

Message primitives perform a **send** operation. Messages first wrap
their arguments in a thunk with the lexical scope (see the next section
for more on thunks), then they perform a **send** operation on the
current receiver.

    Method eval(scope, recv) {
        args = thunk(self args, scope)
        recv send(self name, args)
    }

Expressions are evaluated by evaluating the root, and then
evaluating each subsequent message using the previous result as the
current receiver.

    Expr eval(scope, recv) {
        recv = var(recv)
        for(msg, self) do(
            recv := msg eval(scope, recv)
        )
        recv
    }


Vanilla objects are evaluated slot by slot. We clone the current scope,
setting `this` to be our evaluated object.
    
    Object eval(scope, recv) {
        scope = scope clone
        scope this = ()
        for(slot, self) do(
            evalSlot(slot, scope)
        )
        scope this
    }

    evalSlot(slot, scope) {
        value = slot eval(scope, scope)
        if(not(value nil?)
           scope this append(value))
    }

### 4.4 Thunks

