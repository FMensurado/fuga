#!/usr/bin/env python2.6

from doctest import testmod

class PEG(object):
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
            if name[:2] == 'm_':
                self._matcher[name[2:]] = PE(getattr(self, name))

    def _reset(self):
        self._memo = {}

    def _match(self, name, code, index):
        if (name,index) in self._memo:
            return self._memo[(name,index)]
        match = self._matcher[name](code, index, self)
        self._memo[(name,index)] = match
        return match

    def parse(self, code):
        self._reset()
        match = self._match('start', code, 0)
        self._reset()
        if match.success:
            return match.value
        else:
            raise SyntaxError, match.errormsg()

class Match(object):
    def __init__(self, success, index, value):
        if not isinstance(success, bool):
            raise TypeError, "success must be a bool"
        if not isinstance(index, int):
            raise TypeError, "index must be an int"
        self.success = success
        self.index   = index
        self.value   = value

    def __add__(self, other):
        """Combine two failed matches.

        To combine two failed matches, you either return whichever has
        the highest index or, in case they have the same index, you
        must return the union of their "value"s.
        
        >>> Match(False, 0, [0]) + Match(False, 1, [1])
        Match(False, 1, [1])
        >>> Match(False, 1, [0]) + Match(False, 0, [1])
        Match(False, 1, [0])
        >>> Match(False, 0, [0]) + Match(False, 0, [1])
        Match(False, 0, [0, 1])
        >>> Match(False, 0, [0,1]) + Match(False, 0, [1,2])
        Match(False, 0, [0, 1, 2])
        >>> try: Match(True, 0, [0]) + Match(True, 1, [1])
        ... except TypeError, e: pass
        >>> try: Match(True, 0, [0]) + Match(False, 1, [1])
        ... except TypeError, e: pass
        >>> try: Match(False, 0, [0]) + Match(True, 1, [1])
        ... except TypeError, e: pass
        """
        if self.success or other.success:
            raise TypeError, "can only combine _failed_ matches"
        if self.index > other.index:
            return self
        if other.index > self.index:
            return other
        combined = list(set(self.value) | set(other.value))
        return Match(False, self.index, combined)


    def __repr__(self):
        return "Match(%r, %r, %r)" % (self.success, self.index, self.value)

    def errormsg(self):
        return "expected %s" ', '.join([repr(c) for c in self.value])


class Matcher(object):
    def __call__(self, code, index, grammar):
        return self.match(code, index, grammar)

    def star(self):
        return _star(self)

    def plus(self):
        return _plus(self)

    def opt(self):
        return _opt(self)

    def __div__(self, other):
        return _or(self, other)

class sym(Matcher):
    """Match to the given symbol in the enclosing grammar.
    """
    def __init__(self, name):
        self.name = name

    def __call__(self, code, index, grammar):
        return grammar._match(self.name, code, index)

class lit(Matcher):
    """Match a literal string.
    
    >>> lit("10").match("010", 0, None)
    Match(False, 0, ['10'])
    >>> lit("10").match("010", 3, None)
    Match(False, 3, ['10'])
    >>> lit("10").match("010", 1, None)
    Match(True, 3, '10')
    >>> lit("10").match("1010", 0, None)
    Match(True, 2, '10')
    >>> lit("abc").match("1010", 0, None)
    Match(False, 0, ['abc'])
    """
    def __init__(self, string):
        self.string = string

    def match(self, code, index, grammar):
        endindex = index + len(self.string)
        if self.string == code[index:endindex]:
            return Match(True, endindex, self.string)
        else:
            return Match(False, index, [self.string])

class dot(Matcher):
    """Match any single character.
    
    >>> dot().match('10', 0, None)
    Match(True, 1, '1')
    >>> dot().match('10', 1, None)
    Match(True, 2, '0')
    >>> dot().match('10', 2, None)
    Match(False, 2, [])
    """
    def match(self, code, index, grammar):
        if index < len(code):
            return Match(True, index+1, code[index])
        else:
            return Match(False, index, [])

class cc(Matcher):
    r"""Matches a character class.
    
    >>> cc('ab').match('ab', 0, None)
    Match(True, 1, 'a')
    >>> cc('ab').match('bb', 0, None)
    Match(True, 1, 'b')
    >>> cc('ab').match('cb', 0, None)
    Match(False, 0, ['[ab]'])
    >>> cc('a-z').match('cb', 0, None)
    Match(True, 1, 'c')
    >>> cc('a-z').match('z', 0, None)
    Match(True, 1, 'z')
    >>> cc(r'\n').match('\n', 0, None)
    Match(True, 1, '\n')
    >>> cc(r'\n').match(r'\n', 0, None)
    Match(False, 0, ['[\\n]'])
    >>> cc('abc').match('', 0, None)
    Match(False, 0, ['[abc]'])
    """
    def __init__(self, desc):
        self.desc = desc
        self.charset = set()
        
        i = 0
        while i < len(desc):
            if desc[i] == '\\':
                i += 1
                if i >= len(desc):
                    raise ValueError, "missing escaped character"
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

    def match(self, code, index, grammar):
        if index >= len(code):
            return Match(False, index, ['[%s]' % self.desc])
        elif code[index] in self.charset:
            return Match(True, index+1, code[index])
        else:
            return Match(False, index, ['[%s]' % self.desc])

class _star(Matcher):
    """Match zero or more of another matcher.
    
    >>> lit("a").star().match("aaaa", 0, None)
    Match(True, 4, ['a', 'a', 'a', 'a'])
    >>> lit("a").star().match(" aaa", 0, None)
    Match(True, 0, [])
    >>> lit("a").star().match(" aab", 1, None)
    Match(True, 3, ['a', 'a'])
    >>> lit("aa").star().match("aaaaaa", 0, None)
    Match(True, 6, ['aa', 'aa', 'aa'])
    >>> lit("ab").star().match("ababaa", 0, None)
    Match(True, 4, ['ab', 'ab'])
    >>> lit("ab").star().match("ababaa", 2, None)
    Match(True, 4, ['ab'])
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def match(self, code, index, grammar):
        results = []
        match = self.matcher(code, index, grammar)
        while match.success:
            index = match.index
            results.append(match.value)
            match = self.matcher(code, index, grammar)
        return Match(True, index, results)

class _plus(Matcher):
    """Match one or more of another matcher.
    
    >>> lit("a").plus().match("aaaa", 0, None)
    Match(True, 4, ['a', 'a', 'a', 'a'])
    >>> lit("a").plus().match(" aaa", 0, None)
    Match(False, 0, ['a'])
    >>> lit("a").plus().match(" aab", 1, None)
    Match(True, 3, ['a', 'a'])
    >>> lit("aa").plus().match("aaaaaa", 0, None)
    Match(True, 6, ['aa', 'aa', 'aa'])
    >>> lit("ab").plus().match("ababaa", 0, None)
    Match(True, 4, ['ab', 'ab'])
    >>> lit("ab").plus().match("ababaa", 2, None)
    Match(True, 4, ['ab'])
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def match(self, code, index, grammar):
        results = []
        match = self.matcher(code, index, grammar)
        if not match.success:
            return match
        while match.success:
            index = match.index
            results.append(match.value)
            match = self.matcher(code, index, grammar)
        return Match(True, index, results)

class _opt(Matcher):
    """Match zero or one of another matcher.
    
    >>> lit("a").opt().match("  a ", 1, None)
    Match(True, 1, None)
    >>> lit("a").opt().match("  a ", 2, None)
    Match(True, 3, 'a')
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def match(self, code, index, grammar):
        match = self.matcher(code, index, grammar)
        if match.success:
            return match
        else:
            return Match(True, index, None)

class seq(Matcher):
    """Match a sequence of matchers.

    >>> seq(lit("a"), lit("b")).match(" ab ", 1, None)
    Match(True, 3, ('a', 'b'))
    >>> seq(lit("a"), lit("b")).match(" xb ", 1, None)
    Match(False, 1, ['a'])
    >>> seq(lit("a"), lit("b")).match(" ax ", 1, None)
    Match(False, 2, ['b'])
    """
    def __init__(self, *matchers):
        self.matchers = matchers

    def match(self, code, index, grammar):
        results = []
        for matcher in self.matchers:
            match = matcher(code, index, grammar)
            if match.success:
                results.append(match.value)
                index = match.index
            else:
                return match
        return Match(True, index, tuple(results))

class _or(Matcher):
    """Match any of two matchers, preferrably the first one.

    >>> (lit("a")/lit("b")).match('ab', 0, None)
    Match(True, 1, 'a')
    >>> (lit("a")/lit("b")).match('ab', 1, None)
    Match(True, 2, 'b')
    >>> (lit("a")/lit("ab")).match('ab', 0, None)
    Match(True, 1, 'a')
    >>> (lit("ab")/lit("a")).match('ab', 0, None)
    Match(True, 2, 'ab')
    >>> (lit("a")/lit("b")).match('x', 0, None)
    Match(False, 0, ['a', 'b'])
    >>> (lit("a")/seq(lit("x"), lit("b"))).match('x', 0, None)
    Match(False, 1, ['b'])
    """
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def match(self, code, index, grammar):
        leftm = self.left.match(code, index, grammar)
        if leftm.success:
            return leftm
        rightm = self.right.match(code, index, grammar)
        if rightm.success:
            return rightm
        return leftm + rightm

class andp(Matcher):
    """Match a matcher without consuming input.
    
    >>> andp(lit("a")).match("a", 0, None)
    Match(True, 0, 'a')
    >>> andp(lit("a")).match("a", 1, None)
    Match(False, 1, ['a'])
    >>> andp(lit("a")).match("b", 0, None)
    Match(False, 0, ['a'])
    >>> andp(seq(lit("a"), lit("b"))).match("ax", 0, None)
    Match(False, 1, ['b'])
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def match(self, code, index, grammar):
        match = self.matcher(code, index, grammar)
        if match.success:
            match.index = index
        return match


class notp(Matcher):
    """Match a matcher, but expect it to fail. Consumes no input.

    >>> notp(lit("a")).match("a", 0, None)
    Match(False, 0, [])
    >>> notp(lit("a")).match(" a", 1, None)
    Match(False, 1, [])
    >>> notp(lit("a")).match("a", 1, None)
    Match(True, 1, None)
    >>> notp(lit("a")).match("b", 0, None)
    Match(True, 0, None)
    """
    def __init__(self, matcher):
        self.matcher = matcher

    def match(self, code, index, grammar):
        match = self.matcher(code, index, grammar)
        if match.success:
            return Match(False, index, [])
        else:
            return Match(True, index, None)


def PE(matcher):
    if isinstance(matcher, Matcher):
        return matcher
    raise TypeError, "expected a Matcher"

if __name__ == '__main__':
   testmod()

