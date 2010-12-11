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
        if match.success and hasattr(self, name):
            match.value = getattr(self, name)(match.value)
        self._memo[(name,index)] = match
        return match

    def parse(self, code, name='start'):
        self._reset()
        match = self._match(name, code, 0)
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
        return "expected %s" % ', '.join([repr(c) for c in self.value])


class Matcher(object):
    def __call__(self, code, index, grammar):
        return self.match(code, index, grammar)

    def reprnp(self):
        return repr(self)

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

    def __repr__(self):
        return 'sym(%r)' % self.name

    def match(self, code, index, grammar):
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

    def __repr__(self):
        return "lit(%r)" % self.string

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
    def __repr__(self):
        return "dot()"

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

    def __repr__(self):
        return "cc(%r)" % self.desc

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

    def __repr__(self):
        return '%r.star()' % self.matcher

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

    def __repr__(self):
        return '%r.plus()' % self.matcher

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

    def __repr__(self):
        return repr(self.matcher) + '.opt()'

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

    def __repr__(self):
        return 'seq(%s)'%', '.join([v.reprnp() for v in self.matchers])

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

    def reprnp(self):
        return '%s / %s' % (self.left.reprnp(), self.right.reprnp())

    def __repr__(self):
        return '(%s)' % self.reprnp()

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

    def __repr__(self):
        return 'andp(%s)' % self.matcher.reprnp()

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

    def __repr__(self):
        return 'notp(%s)' % self.matcher.reprnp()

    def match(self, code, index, grammar):
        match = self.matcher(code, index, grammar)
        if match.success:
            return Match(False, index, [])
        else:
            return Match(True, index, None)


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
    m_Class      = seq(lit('['), seq(notp(lit(']')), sym("Range")).star(),
                       lit(']'), sym("Spacing"))

    m_Range      = (seq(sym('Char'), lit('-'), sym('Char')) /
                    seq(sym('Char')))

    m_Char       = (seq(lit("\\"), cc('\n\t\r\a\'\"\[\]\\-')) /
                    seq(notp(lit("\\")), dot()))


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
    raise TypeError, "expected a Matcher"


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

