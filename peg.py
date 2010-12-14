#!/usr/bin/env python3

import sys
from doctest import testmod
from functools import reduce

TABSTOP = 4

class Location:
    """Location(filename, line, column) => location
    Represent a location in code -- filename, line, and column.
    This object is immutable and persistant. That is, operations
    that involve this object cannot modify the object, they can
    only create new versions. This makes it easy to store anywhere
    without fear that it will be modified.
    """
    def __init__(self, filename, line=1, column=1):
        assert isinstance(filename, str)
        assert isinstance(line, int)
        assert isinstance(column, int)
        object.__setattr__(self, 'filename', filename)
        object.__setattr__(self, 'line', line)
        object.__setattr__(self, 'column', column)

    def __setattr__(self, name):
        raise TypeError("self is not mutable")

    def __str__(self):
        return "%s (line %s, column %s)" % (
            self.filename, self.line, self.column
        )

    def __repr__(self):
        return 'Location(%r, %r, %r)' % (
            self.filename, self.line, self.column
        )

    def over(self, code):
        r"""
        Calculate the new location after moving over code.
        
        >>> Location('', 0, 0).over('')
        Location('', 0, 0)
        >>> Location('', 0, 0).over('soprano')
        Location('', 0, 7)
        >>> Location('', 0, 1).over('\t').column == TABSTOP+1
        True
        >>> Location('', 0, 2).over('\t').column == TABSTOP+1
        True
        >>> Location('', 0, 1+TABSTOP).over('\t').column == TABSTOP*2+1
        True
        >>> Location('', 0, 0).over('do\nre')
        Location('', 1, 3)
        >>> Location('', 0, 0).over('do\n\rre')
        Location('', 1, 3)
        >>> Location('', 10, 32).over('do re mi')
        Location('', 10, 40)
        >>> Location('', 10, 32).over('do\nre\nmi')
        Location('', 12, 3)
        """
        line   = self.line
        column = self.column
        for c in code:
            if c == '\n':
                line   += 1
                column  = 1
            elif c == '\t':
                column  = ((column - 1) // TABSTOP + 1) * TABSTOP + 1
            elif c == '\r':
                pass
            else:
                column += 1
        return Location(self.filename, line, column)

    def __eq__(self, other):
        return (self.filename == other.filename and
                self.line     == other.line     and
                self.column   == other.column)

    def __lt__(self, other):
        """
        >>> try: Location('', 0, 0) < 10
        ... except TypeError as e: print(e)
        incompatible types for '<': peg.Location and int
        >>> try: Location('foo', 0,0) < Location('bar', 0,0)
        ... except ValueError as e: print(e)
        can't compare locations in different files
        >>> Location('', 0, 1) < Location('', 1, 0)
        True
        >>> Location('', 1, 0) < Location('', 0, 1)
        False
        >>> Location('', 0, 0) < Location('', 0, 0)
        False
        >>> Location('', 0, 0) < Location('', 0, 1)
        True
        >>> Location('', 0, 1) < Location('', 0, 0)
        False
        """
        if not isinstance(other, Location):
            raise TypeError(
                "incompatible types for '<': peg.Location and " + 
                    type(other).__name__
            )
        if self.filename != other.filename:
            raise ValueError("can't compare locations in different files")
        if self.line < other.line:
            return True
        if self.line > other.line:
            return False
        return self.column < other.column

class Iterator:
    """
    Iterator keeps track of location while traversing through code.
    """
    __slots__ = ['code', 'index', 'location']
    def __init__(self, code, filename="<file>"):
        assert isinstance(code, str)
        assert isinstance(filename, str)
        self.code  = code
        self.index = 0
        self.location = Location(filename)

    def match(self, pattern):
        """
        Iterator.match(self, pattern) => bool
        Determine whether the code at the current location matches the 
        given pattern. The pattern can be either a string, or a set of
        characters.
        
        >>> iter = Iterator('a b c d e f g', '')
        >>> iter.match('')
        True
        >>> iter.match('a b c')
        True
        >>> iter.match('b c d')
        False
        >>> iter.match(set([]))
        False
        >>> iter.match(set(['a', 'b', 'c']))
        True
        >>> iter.match(set(['A', 'b', 'c']))
        False
        >>> iter.match(set(['a', 'B', 'c']))
        True
        >>> iter.index = 2
        >>> iter.match('a b c')
        False
        >>> iter.match('b c d')
        True
        >>> iter.match(set(['a', 'b', 'c']))
        True
        >>> iter.match(set(['A', 'b', 'c']))
        True
        >>> iter.match(set(['a', 'B', 'c']))
        False
        >>> iter = Iterator('', '')
        >>> iter.match('do re mi')
        False
        >>> iter.match(set(['d', 'r', 'm']))
        False
        """
        if isinstance(pattern, str):
            endix = self.index + len(pattern)
            return self.code[self.index:endix] == pattern
        elif isinstance(pattern, set):
            return (self.index < len(self.code)
               and  self.code[self.index] in pattern)
        else:
            raise TypeError("pattern must be a string or a set")
    
    def move(self, n):
        r"""
        Iterator.move(self, n) => None
        
        >>> iter = Iterator('do re mi', '')
        >>> iter.index
        0
        >>> iter.move(2)
        >>> iter.index
        2
        >>> iter.move(2); iter.index
        4
        >>> iter.move(4); iter.index
        8
        >>> try: iter.move(2); iter.index
        ... except IndexError as e: print(e)
        Can't move beyond end of code.
        >>> iter = Iterator('do\nre mi', '')
        >>> iter.location
        Location('', 1, 1)
        >>> iter.move(2); iter.location
        Location('', 1, 3)
        >>> iter.move(2); iter.location
        Location('', 2, 2)
        """
        endix = self.index + n
        if endix > len(self.code):
            raise IndexError("Can't move beyond end of code.")
        self.location = self.location.over(self.code[self.index:endix])
        self.index = endix

    def clone(self):
        r"""
        Iterator.clone(self) => iterator
        Create a new iterator based on self. Modifications to either
        iterator will affect only one of them.
        
        >>> iter1 = Iterator('do re mi', '')
        >>> iter1.move(2)
        >>> iter2 = iter1.clone()
        >>> iter2.index == iter1.index
        True
        >>> iter2.location == iter1.location
        True
        >>> iter2.move(2)
        >>> iter2.index == iter1.index
        False
        >>> iter2.location == iter1.location
        False
        """
        iter = Iterator(self.code)
        iter.copy(self)
        return iter

    def copy(self, other):
        self.code     = other.code
        self.index    = other.index
        self.location = other.location

class PEG:
    """
    
    >>> class test1(PEG):
    ...     m_start = lit("a")
    ... 
    >>> test1().parse('a')
    'a'
    >>> try: test1().parse('0')
    ... except SyntaxError: pass
    >>>
    >>> class test2(PEG):
    ...     m_start = sym("a")
    ...     m_a = lit("a")
    ...
    >>> test2().parse('a')
    'a'
    >>> try: test2().parse('0')
    ... except SyntaxError: pass
    """
    def __init__(self):
        if type(self) is PEG:
            return
        self._matcher = {}
        for name in dir(self):
            if name[:2] == 'm_' or name[:2] == 'i_':
                self._matcher[name[2:]] = PE(getattr(self, name))

    def _match(self, name, iter):
        index = iter.index
        if (name, index) in self._memo:
            (match, iterc) = self._memo[(name, index)]
            iter.copy(iterc)
            return match
        match = self._matcher[name](iter, self)
        if match.success:
            if hasattr(self, name):
                match.value = getattr(self, name)(match.value)
        self._memo[(name, index)] = (match, iter.clone())
        return match

    def _expected(self, match, iter):
        if not self._expecting or not hasattr(self, 'm_'+match.value[0]):
            return
        if self._worst is None:
            self._worst = match
            self._worstIndex = iter.index
        else:
            self._worst += match
            self._worstIndex = max(iter.index, self._worstIndex)
    
    def _toggleExpecting(self):
        self._expecting = not self._expecting

    def parse(self, code, name='start', filename='<file>'):
        self._memo = {}
        self._worst = None
        self._worstIndex = 0
        self._expecting = True
        match = self._match(name, Iterator(code, filename))
        del self._memo
        if match.success:
            return match.value
        else:
            raise SyntaxError(self._worst.errormsg())

    def worst(self):
        return self._worst

    def worstIndex(self):
        return self._worstIndex

class Match(object):
    def __init__(self, success, location, value):
        if not isinstance(success, bool):
            raise TypeError("success must be a bool")
        if not isinstance(location, Location):
            raise TypeError("index must be an int")
        self.success  = success
        self.location = location
        self.value    = value

    def __add__(self, other):
        """Combine two failed matches.

        To combine two failed matches, you either return whichever has
        the highest index or, in case they have the same index, you
        must return the union of their "value"s.
        
        >>> l0 = Location('',0,0)
        >>> l1 = Location('',0,1)
        >>> Match(False, l0, [0])  + Match(False, l1, [1])
        Match(False, Location('', 0, 1), [1])
        >>> Match(False, l1, [0])  + Match(False, l0, [1])
        Match(False, Location('', 0, 1), [0])
        >>> Match(False, l0, [0]) + Match(False, l0, [1])
        Match(False, Location('', 0, 0), [0, 1])
        >>> Match(False, l0, [0,1]) + Match(False, l0, [1,2])
        Match(False, Location('', 0, 0), [0, 1, 2])
        >>> try: Match(True, l0, [0]) + Match(True, l1, [1])
        ... except TypeError as e: pass
        >>> try: Match(True, l0, [0]) + Match(False, l1, [1])
        ... except TypeError as e: pass
        >>> try: Match(False, l0, [0]) + Match(True, l1, [1])
        ... except TypeError as e: pass
        """
        if self.success or other.success:
            raise TypeError("can only combine failed matches")
        if self.location < other.location:
            return other
        if other.location < self.location:
            return self
        combined = list(set(self.value) | set(other.value))
        return Match(False, self.location, combined)


    def __repr__(self):
        return "Match(%r, %r, %r)" % (
            self.success, self.location, self.value
        )

    def errormsg(self):
        if self.value:
            return "SYNTAX ERROR in %s: Expected %s." % (
                self.location,
                ', '.join(self.value)
            )
        else:
            return "SYNTAX ERROR in %s." % self.location


class Matcher(object):
    def __call__(self, iter, grammar):
        return self.match(iter, grammar)

    def reprnp(self):
        return repr(self)

    def star(self):
        return _star(self)

    def plus(self):
        return _plus(self)

    def opt(self):
        return _opt(self)

    def __truediv__(self, other):
        return _or(self, other)

class sym(Matcher):
    """Match to the given symbol in the enclosing grammar.
    """
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return 'sym(%r)' % self.name

    def match(self, iter, grammar):
        location = iter.location
        match    = grammar._match(self.name, iter)
        if (not match.success) and location == match.location:
            match = Match(False, location, [self.name])
            grammar._expected(match, iter)
            return match
        else:
            return match

class lit(Matcher):
    """Match a literal string.
    
    >>> iter = Iterator("010", '')
    >>> lit("10").match(iter, None)
    Match(False, Location('', 1, 1), ["'10'"])
    >>> iter.index
    0
    >>> iter.move(1)
    >>> lit("10").match(iter, None)
    Match(True, Location('', 1, 2), '10')
    >>> iter.index
    3
    """
    def __init__(self, string):
        self.string = string

    def __repr__(self):
        return "lit(%r)" % self.string

    def match(self, iter, grammar):
        location = iter.location
        if iter.match(self.string):
            iter.move(len(self.string))
            return Match(True,  location,  self.string)
        else:
            return Match(False, location, [repr(self.string)])

class dot(Matcher):
    """Match any single character.
    
    >>> iter = Iterator('10', '')
    >>> dot().match(iter, None)
    Match(True, Location('', 1, 1), '1')
    >>> dot().match(iter, None)
    Match(True, Location('', 1, 2), '0')
    >>> dot().match(iter, None)
    Match(False, Location('', 1, 3), ['.'])
    """
    def __repr__(self):
        return "dot()"

    def match(self, iter, grammar):
        location = iter.location
        if iter.index < len(iter.code):
            char = iter.code[iter.index]
            iter.move(1)
            return Match(True,  location, char)
        else:
            return Match(False, location, ['.'])

class cc(Matcher):
    r"""Matches a character class.
    
    >>> iter = Iterator('abc', '')
    >>> cc('ab').match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> cc('ab').match(iter, None)
    Match(True, Location('', 1, 2), 'b')
    >>> cc('ab').match(iter, None)
    Match(False, Location('', 1, 3), ['[ab]'])
    >>> iter = Iterator('', '')
    >>> cc('ab').match(iter, None)
    Match(False, Location('', 1, 1), ['[ab]'])
    >>> iter = Iterator('cb', '')
    >>> cc('a-z').match(iter, None)
    Match(True, Location('', 1, 1), 'c')
    >>> iter = Iterator('z', '')
    >>> cc('a-z').match(iter, None)
    Match(True, Location('', 1, 1), 'z')
    >>> iter = Iterator('\n\\n', '')
    >>> cc(r'\n').match(iter, None)
    Match(True, Location('', 1, 1), '\n')
    >>> cc(r'\n').match(iter, None)
    Match(False, Location('', 2, 1), ['[\\n]'])
    """
    def __init__(self, desc):
        self.desc = desc
        self.charset = set()
        
        i = 0
        while i < len(desc):
            if desc[i] == '\\':
                i += 1
                if i >= len(desc):
                    raise ValueError("missing escaped character")
                escapes = {
                    'n': '\n',
                    'r': '\n',
                    't': '\t'
                }
                char = escapes.get(desc[i], desc[i])
                self.charset.add(char)
            elif desc[i] == '-':
                for c in range(ord(desc[i-1]),  ord(desc[i+1])):
                    self.charset.add(chr(c))
            else:
                self.charset.add(desc[i])
            i += 1

    def __repr__(self):
        return "cc(%r)" % self.desc

    def match(self, iter, grammar):
        location = iter.location
        if iter.match(self.charset):
            iter.move(1)
            return Match(True,  location, iter.code[iter.index-1])
        else:
            return Match(False, location, ['[%s]' % self.desc])

class _star(Matcher):
    """Match zero or more of another matcher.
    
    >>> iter = Iterator('aaaa', '')
    >>> lit("a").star().match(iter, None)
    Match(True, Location('', 1, 1), ['a', 'a', 'a', 'a'])
    >>> iter = Iterator(' aaa', '')
    >>> lit("a").star().match(iter, None)
    Match(True, Location('', 1, 1), [])
    >>> iter = Iterator('aab', '')
    >>> iter.move(1)
    >>> lit("a").star().match(iter, None)
    Match(True, Location('', 1, 2), ['a'])
    >>> iter = Iterator('aaaaaa', '')
    >>> lit("aa").star().match(iter, None)
    Match(True, Location('', 1, 1), ['aa', 'aa', 'aa'])
    >>> iter = Iterator('ababaa', '')
    >>> lit("ab").star().match(iter, None)
    Match(True, Location('', 1, 1), ['ab', 'ab'])
    >>> iter = Iterator('ababacab', '')
    >>> seq(lit("a"), lit("b")).star().match(iter, None)
    Match(True, Location('', 1, 1), [('a', 'b'), ('a', 'b')])
    >>> iter.index
    4
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def __repr__(self):
        return '%r.star()' % self.matcher

    def match(self, iter, grammar):
        results = []
        location = iter.location
        match = self.matcher(iter, grammar)
        while match.success:
            results.append(match.value)
            match = self.matcher(iter, grammar)
        return Match(True, location, results)

class _plus(Matcher):
    """Match one or more of another matcher.
    
    >>> lit("a").plus().match(Iterator('aaaa', ''), None)
    Match(True, Location('', 1, 1), ['a', 'a', 'a', 'a'])
    >>> lit("a").plus().match(Iterator(" aaa", ''), None)
    Match(False, Location('', 1, 1), ["'a'"])
    >>> lit("a").plus().match(Iterator("ab", ''), None)
    Match(True, Location('', 1, 1), ['a'])
    >>> lit("a").plus().match(Iterator("aab", ''), None)
    Match(True, Location('', 1, 1), ['a', 'a'])
    >>> lit("aa").plus().match(Iterator("aaaaaa", ''), None)
    Match(True, Location('', 1, 1), ['aa', 'aa', 'aa'])
    >>> lit("ab").plus().match(Iterator("ababaa", ''), None)
    Match(True, Location('', 1, 1), ['ab', 'ab'])
    >>> iter = Iterator('ababacab', '')
    >>> seq(lit("a"), lit("b")).plus().match(iter, None)
    Match(True, Location('', 1, 1), [('a', 'b'), ('a', 'b')])
    >>> iter.index
    4
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def __repr__(self):
        return '%r.plus()' % self.matcher

    def match(self, iter, grammar):
        results = []
        location = iter.location
        match = self.matcher(iter, grammar)
        if not match.success:
            return match
        while match.success:
            results.append(match.value)
            match = self.matcher(iter, grammar)
        return Match(True, location, results)


class _opt(Matcher):
    """Match zero or one of another matcher.
    
    >>> iter = Iterator("  a ", '')
    >>> lit("a").opt().match(iter, None)
    Match(True, Location('', 1, 1), None)
    >>> iter.move(2)
    >>> lit("a").opt().match(iter, None)
    Match(True, Location('', 1, 3), 'a')
    >>> iter = Iterator('acabacab', '')
    >>> seq(lit("a"), lit("b")).opt().match(iter, None)
    Match(True, Location('', 1, 1), None)
    >>> iter.index
    0
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def __repr__(self):
        return repr(self.matcher) + '.opt()'

    def match(self, iter, grammar):
        location = iter.location
        match = self.matcher(iter, grammar)
        if match.success:
            return match
        else:
            return Match(True, location, None)

class seq(Matcher):
    """Match a sequence of matchers.

    >>> iter = Iterator('ab', '')
    >>> seq(lit("a"), lit("b")).match(iter, None)
    Match(True, Location('', 1, 1), ('a', 'b'))
    >>> iter.index
    2
    >>> iter = Iterator('xb', '')
    >>> seq(lit("a"), lit("b")).match(iter, None)
    Match(False, Location('', 1, 1), ["'a'"])
    >>> iter.index
    0
    >>> iter = Iterator('ax', '')
    >>> seq(lit("a"), lit("b")).match(iter, None)
    Match(False, Location('', 1, 2), ["'b'"])
    >>> iter.index
    0
    >>> iter = Iterator('a', '')
    >>> seq(lit("a"), lit("b")).match(iter, None)
    Match(False, Location('', 1, 2), ["'b'"])
    >>> iter = Iterator('', '')
    >>> seq(lit("a"), lit("b")).match(iter, None)
    Match(False, Location('', 1, 1), ["'a'"])
    """
    def __init__(self, *matchers):
        self.matchers = matchers

    def __repr__(self):
        return 'seq(%s)'%', '.join([v.reprnp() for v in self.matchers])

    def match(self, iter, grammar):
        location = iter.location
        iterc = iter.clone()
        results = []
        for matcher in self.matchers:
            match = matcher(iterc, grammar)
            if not match.success:
                return match
            results.append(match.value)
        iter.copy(iterc)
        return Match(True, location, tuple(results))

class _or(Matcher):
    """Match any of two matchers, preferrably the first one.

    >>> iter = Iterator('ab', '')
    >>> (lit("a")/lit("b")).match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> (lit("a")/lit("b")).match(iter, None)
    Match(True, Location('', 1, 2), 'b')
    >>> (lit("a")/lit("b")).match(iter, None)
    Match(False, Location('', 1, 3), ["'a'", "'b'"])
    >>> iter = Iterator('', '')
    >>> (lit("a")/lit("b")).match(iter, None)
    Match(False, Location('', 1, 1), ["'a'", "'b'"])
    >>> iter.index
    0
    >>> iter = Iterator('ab', '')
    >>> (lit("a")/lit("ab")).match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> iter = Iterator('ab', '')
    >>> (lit("ab")/lit("a")).match(iter, None)
    Match(True, Location('', 1, 1), 'ab')
    >>> iter = Iterator('ab', '')
    >>> (seq(lit("a"), lit("c")) / lit("ab")).match(iter, None)
    Match(True, Location('', 1, 1), 'ab')
    """
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def reprnp(self):
        return '%s / %s' % (self.left.reprnp(), self.right.reprnp())

    def __repr__(self):
        return '(%s)' % self.reprnp()

    def match(self, iter, grammar):
        leftm = self.left(iter, grammar)
        if leftm.success:
            return leftm
        rightm = self.right(iter, grammar)
        if rightm.success:
            return rightm
        return leftm + rightm

class andp(Matcher):
    """Match a matcher without consuming input.
    
    >>> iter = Iterator('a', '')
    >>> andp(lit("a")).match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> andp(lit("a")).match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> andp(lit("a")).match(iter, None)
    Match(True, Location('', 1, 1), 'a')
    >>> andp(lit("a")).match(Iterator("", ''), None)
    Match(False, Location('', 1, 1), ["'a'"])
    >>> andp(lit("a")).match(Iterator("b", ''), None)
    Match(False, Location('', 1, 1), ["'a'"])
    >>> andp(seq(lit("a"), lit("b"))).match(Iterator("ax", ''), None)
    Match(False, Location('', 1, 2), ["'b'"])
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def __repr__(self):
        return 'andp(%s)' % self.matcher.reprnp()

    def match(self, iter, grammar):
        iter = iter.clone()
        return self.matcher(iter, grammar)

class notp(Matcher):
    """Match a matcher, but expect it to fail. Consumes no input.

    >>> iter = Iterator('a', '')
    >>> notp(lit("a")).match(iter, None)
    Match(False, Location('', 1, 1), [])
    >>> notp(lit("a")).match(iter, None)
    Match(False, Location('', 1, 1), [])
    >>> notp(lit("a")).match(Iterator("", ''), None)
    Match(True, Location('', 1, 1), None)
    >>> notp(lit("a")).match(Iterator("b", ''), None)
    Match(True, Location('', 1, 1), None)
    >>> iter = Iterator('ax', '')
    >>> notp(seq(lit("a"), lit("b"))).match(iter, None)
    Match(True, Location('', 1, 1), None)
    >>> iter.index
    0
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def __repr__(self):
        return 'notp(%s)' % self.matcher.reprnp()

    def match(self, iter, grammar):
        iter  = iter.clone()
        location = iter.location
        grammar._toggleExpecting()
        match = self.matcher(iter, grammar)
        grammar._toggleExpecting()
        if match.success:
            return Match(False, location, [])
        else:
            return Match(True, location, None)

class PE_parser(PEG):
    m_start = seq(sym('Expression'), sym('EndOfFile'))

    def start(self, v):
        return v[0]

    # Hierarchical
    m_Expression = seq(sym('Sequence'),
                       seq(sym('SLASH'),
                           sym('Sequence')).star())
    m_Sequence   = sym('Prefix').star()
    m_Prefix     = seq((sym('AND') / sym('NOT')).opt(),
                       sym('Suffix'))
    m_Suffix     = seq(sym('Primary'),
                       (sym('QUESTION') / sym('STAR') / sym('PLUS')).opt())
    m_Primary    = (sym('Identifier') /
                    seq(sym('OPEN'), sym('Expression'), sym('CLOSE')) /
                    sym('Literal') / sym('Class') / sym('DOT'))

    def Expression(self, v):
        result = v[0]
        for w in v[1]:
            result = result / w[1]
        return result

    def Sequence(self, v):
        if len(v) == 1:
            return v[0]
        else:
            return seq(*v)
    
    def Prefix(self, v):
        if v[0] is None:
            return v[1]
        prefix = {'&': andp, '!': notp}
        return prefix[v[0][0]](v[1])

    def Suffix(self, v):
        if v[1] is None:
            return v[0]
        suffix = {'?': 'opt', '*': 'star', '+': 'plus'}
        return getattr(v[0], suffix[v[1][0]])()

    def Primary(self, v):
        if isinstance(v, tuple):
            return v[1]
        else:
            return v
   
    # Lexical
    m_Identifier = seq(cc('a-zA-Z_'),
                       cc('a-zA-Z_0-9').star(),
                       sym('Spacing'))

    m_Literal    = (seq(lit("'"), seq(notp(lit("'")), sym("Char")).star(),
                        lit("'"), sym("Spacing")) /
                    seq(lit('"'), seq(notp(lit('"')), sym("Char")).star(),
                        lit('"'), sym("Spacing")))
    m_Class      = seq(lit('['), seq(notp(lit(']')), sym('Range')).star(),
                       lit(']'), sym("Spacing"))

    m_Range      = (seq(sym('Char'), lit('-'), sym('Char')) /
                    seq(sym('Char')))

    m_Char       = (seq(lit("\\"), dot()) /
                    seq(notp(lit("\\")), dot()))

    def classstar(self, v):
        return v

    def classseq(self, v):
        return v

    def sentinel(self, v):
        return v

    def Identifier(self, v):
        return sym(v[0] + ''.join(v[1]))

    def Literal(self, v):
        return lit(''.join([w[1] for w in v[1]]))

    def Class(self, v):
        return cc(''.join([w[1] for w in v[1]]))

    def Range(self, v):
        return ''.join(v)

    def Char(self, v):
        escapes = { 'n': '\n', 'r': '\r', 't': '\t',
                    "'": "'",  '"': '"', '[': '[', ']': ']',
                    '\\': '\\', '-': '\-' }
        return escapes.get(v[1], v[1])
    
    m_SLASH      = seq(lit('/'), sym('Spacing'))
    m_AND        = seq(lit('&'), sym('Spacing'))
    m_NOT        = seq(lit('!'), sym('Spacing'))
    m_QUESTION   = seq(lit('?'), sym('Spacing'))
    m_STAR       = seq(lit('*'), sym('Spacing'))
    m_PLUS       = seq(lit('+'), sym('Spacing'))
    m_OPEN       = seq(lit('('), sym('Spacing'))
    m_CLOSE      = seq(lit(')'), sym('Spacing'))
    m_DOT        = seq(lit('.'), sym('Spacing'))

    def DOT(self, v):
        return dot()

    m_Spacing    = cc(' \t\r\n').star()
    m_EndOfFile  = notp(dot())

def PE(matcher):
    r"""
    >>> PE('a')
    sym('a')
    >>> PE('a b c')
    seq(sym('a'), sym('b'), sym('c'))
    >>> PE('')
    seq()
    >>> PE('a?')
    sym('a').opt()
    >>> PE('a*')
    sym('a').star()
    >>> PE('a+')
    sym('a').plus()
    >>> PE('&a')
    andp(sym('a'))
    >>> PE('!a')
    notp(sym('a'))
    >>> PE('a / b')
    (sym('a') / sym('b'))
    >>> PE('a / b / c')
    (sym('a') / sym('b') / sym('c'))
    >>> PE("'abc'")
    lit('abc')
    >>> PE('"abc"')
    lit('abc')
    >>> PE('"abc\\""')
    lit('abc"')
    >>> PE('"\-"')
    lit('\\-')
    >>> PE('.')
    dot()
    >>> PE('[abc]')
    cc('abc')
    >>> PE('[a-zA-Z]')
    cc('a-zA-Z')
    >>> PE('a (b c)*')
    seq(sym('a'), seq(sym('b'), sym('c')).star())
    """
    if isinstance(matcher, Matcher):
        return matcher
    elif isinstance(matcher, str):
        return PE_parser().parse(matcher)
    raise TypeError("expected a Matcher")


class Calc(PEG):
    """
    >>> calc = Calc()
    >>> calc.parse('10')
    10
    >>> calc.parse('1+1')
    2
    >>> calc.parse('5*5')
    25
    >>> calc.parse('1+2*3+4*5')
    27
    >>> calc.parse('(1+2)*(3+4)*5')
    105
    """

    m_start = 'spacing plus eof'
    m_plus  = 'times (PLUS times)*'
    m_times = 'term (TIMES term)*'
    m_term  = 'pexp / num'
    m_pexp  = 'OPEN plus CLOSE'
    m_num   = '[0-9]+ spacing'

    m_PLUS  = '"+" spacing'
    m_TIMES = '"*" spacing'
    m_OPEN  = '"(" spacing'
    m_CLOSE = '")" spacing'

    m_spacing = '[ \t\n\r]*'
    m_eof = r'!.'

    def start(self,v): return v[1]
    def plus(self,v):  return reduce(lambda x,y: x + y[1], v[1], v[0])
    def times(self,v): return reduce(lambda x,y: x * y[1], v[1], v[0])
    def pexp(self,v):  return v[1]
    def num(self,v):   return int(''.join(v[0]))

if __name__ == '__main__':
    testmod()


