#!/usr/bin/env python3

from peg     import PEG
from doctest import testmod
import collections

class FugaError(Exception):
    pass

class fgobj:
    def __init__(self, proto, *vargs, **kwargs):
        self._proto = proto
        self._slots = {}
        self._length = 0
        self._value  = None
        
        if vargs and not isinstance(vargs[0], fgobj):
            self._value = vargs[0]
            vargs = vargs[1:]

        for i, varg in enumerate(vargs):
            self.set(i, varg)
        for k in kwargs:
            self.set(k, kwargs[k])

    def need(self):
        pass

    def proto(self):
        """
        >>> Object = fgobj(None)
        >>> fgobj(Object).proto() is Object
        True
        >>> try: Object.proto()
        ... except FugaError, e: print(e)
        Object proto: Object has no prototype
        """
        self.need()
        if self._proto is not None:
            return self._proto
        else:
            raise FugaError("Object proto: Object has no prototype")

    def value(self):
        """
        >>> Object = fgobj(None)
        >>> Object.value()
        >>> soprano = fgobj(Object, 1)
        >>> soprano.value()
        1
        >>> alto = fgobj(soprano, 2)
        >>> alto.value()
        2
        >>> tenor = fgobj(soprano)
        >>> tenor.value()
        1
        >>> bass = fgobj(tenor, alto)
        >>> bass.value()
        1
        """
        self.need()
        if self._value is not None:
            return self._value
        elif self._proto is not None:
            return self._proto.value()
        else:
            return None

    def rawHas(self, name):
        """
        >>> Object  = fgobj(None)
        >>> soprano = fgobj(Object, soprano=Object)
        >>> alto    = fgobj(soprano)
        >>> tenor   = fgobj(Object, alto)
        >>> bass    = fgobj(tenor)
        >>>
        >>> Object.rawHas('soprano')
        False
        >>> soprano.rawHas('soprano')
        True
        >>> alto.rawHas('soprano')
        False
        >>> tenor.rawHas(0)
        True
        >>> bass.rawHas(0)
        False
        """
        if not (isinstance(name, int) or isinstance(name, str)):
            raise TypeError("expected only int or str for slot name")
        self.need()
        return name in self._slots

    def rawGet(self, name):
        """
        >>> Object  = fgobj(None)
        >>> soprano = fgobj(Object, soprano=Object)
        >>> alto    = fgobj(soprano)
        >>> tenor   = fgobj(Object, alto)
        >>> bass    = fgobj(tenor)
        >>>
        >>> soprano.rawGet('soprano') is Object
        True
        >>> try: alto.rawGet('soprano')
        ... except FugaError, e: print(e)
        Object rawGet: object has no slot soprano
        >>>
        >>> tenor.rawGet(0) is alto
        True
        >>> try: bass.rawGet(0)
        ... except FugaError, e: print(e)
        Object rawGet: object has no slot 0
        """
        if self.rawHas(name):
            return self._slots[name]
        raise FugaError("Object rawGet: object has no slot %s" % name)

    def has(self, name):
        """
        >>> Object  = fgobj(None)
        >>> soprano = fgobj(Object, soprano=Object)
        >>> alto    = fgobj(soprano)
        >>> tenor   = fgobj(Object, alto)
        >>> bass    = fgobj(tenor)
        >>>
        >>> Object.has('soprano')
        False
        >>> soprano.has('soprano')
        True
        >>> alto.has('soprano')
        True
        >>> tenor.has(0)
        True
        >>> bass.has(0)
        False
        """
        if self.rawHas(name):
            return True
        if isinstance(name, str) and not self._proto is None:
            return self._proto.has(name)
        return False

    def get(self, name):
        """
        >>> Object  = fgobj(None)
        >>> soprano = fgobj(Object, soprano=Object)
        >>> alto    = fgobj(soprano)
        >>> tenor   = fgobj(Object, alto)
        >>> bass    = fgobj(tenor)
        >>>
        >>> soprano.get('soprano') is Object
        True
        >>> alto.get('soprano') is Object
        True
        >>> tenor.get(0) is alto
        True
        >>> try: bass.get(0)
        ... except FugaError, e: print(e)
        Object get: object has no slot 0
        """
        if self.rawHas(name):
            return self._slots[name]
        elif self._proto is not None and isinstance(name, str):
            return self._proto.get(name)
        else:
            raise FugaError("Object get: object has no slot %s" % name)

    def set(self, name, value):
        """
        >>> Object = fgobj(None)
        >>> Object.has(0)
        False
        >>> Object._length
        0
        >>> Object.set(0, Object)
        >>> Object.get(0) is Object
        True
        >>> Object._length
        1
        >>> Object.set(2, Object)
        >>> Object._length
        1
        >>> Object.set(1, Object)
        >>> Object._length
        3
        >>> soprano = fgobj(Object)
        >>> Object.set('me', Object)
        >>> soprano.get('me') is Object
        True
        >>> soprano.set('me', soprano)
        >>> soprano.get('me') is Object
        False
        >>> soprano.get('me') is soprano
        True
        """
        if not (isinstance(name, int) or (isinstance(name, str) and name)):
            raise TypeError("expected only int or str for slot name")
        if not isinstance(value, fgobj):
            raise TypeError("expected a fuga object for slot value")
        self.need()
        if name in self._slots:
            raise FugaError("Object set: object already has slot %s" %name)
        self._slots[name] = value
        while self._length in self._slots:
            self._length += 1

    def __len__(self):
        """
        >>> Object = fgobj(None)
        >>> len(fgobj(Object))
        0
        >>> len(Object)
        0
        >>> Object.set(0, Object)
        >>> len(Object)
        1
        >>> Object.set(2, Object)
        >>> len(Object)
        1
        >>> Object.set(1, Object)
        >>> len(Object)
        3
        >>> len(fgobj(Object))
        0
        """
        self.need()
        return self._length

    def append(self, value):
        self.set(len(self), value)

    def __iter__(self):
        """
        Iterate over contiguous indexed slots.

        >>> Object = fgobj(None)
        >>> Object.set(0, Object)
        >>> Object.set(1, Object)
        >>> Object.set(2, Object)
        >>> Object.set(4, Object)
        >>> Object.set('soprano', Object)
        >>> total = 0
        >>> for obj in Object:
        ...     total += 1
        >>> total
        3
        """
        return (self._slots[i] for i in range(len(self))).__iter__()

    def isa(self, proto):
        self.need()
        return (self._proto is not None) and (
            self._proto is proto  or
            self._proto.isa(proto)
        )

    def clone(self, *vargs, **kwargs):
        return fgobj(self, *vargs, **kwargs)

    def eval(self, receiver, scope=None):
        if scope is None:
            scope = receiver
        if self.isa(Int) or self.isa(String) or self.isa(Symbol):
            return self
        elif self.isa(Expr):
            for slot in self:
                receiver = slot.eval(receiver)
            return receiver
        elif self.isa(Quote):
            return self['value']
        elif self.isa(Msg):
            fn = receiver.get(self.value())
            args = Object.clone()
            args._slots = self._slots
            args._length = self._length
            fn.activate(receiver, fgthunk(self, scope))
        else:
            return self.evalSlots(scope)

    def activate(self, receiver, args):
        if self.isa(Method):
            if self.value():
                return self.value()(receiver, args)
            raise FugaError("Method activate: non-primitive methods"
                                              + " not yet supported")
        elif len(args):
            raise FugaError("Object activate: non-method expects"
                                                + " no arguments")
        return self

    def evalSlots(self, scope):
        result = fgthunk(self, scope)
        result.thunkSlots()
        result.needSlots()
        return result

    def __contains__(self, name):
        return self.has(name)

    def __getitem__(self, name):
        return self.get(name)

    def __setitem__(self, name, value):
        self.set(name, value)

    def thunk(self, env):
        return fgthunk(self, env)

    def needSlots(self, scope):
        for slot in self:
            slot.need()

    def strSlots(self, depth=4):
        if not list(self._slots.keys()):
            return fgstr('()')
        if depth == 0:
            return fgstr('(...)')
        slots = []
        for slot in self:
            slots.append(slot.str(depth-1).value())
        for k in self._slots:
            if (isinstance(k, str) and k[0] != '_') or (
                isinstance(k, int) and k > len(self)):
                slots.append('%s=%s' %
                    (k, self._slots[k].str(depth-1).value())
                )
        return fgstr('(%s)' % ', '.join(slots))
    
    def str(self, depth=4):
        assert isinstance(depth, int)
        result = self['str'].activate(self,
            Object.clone(depth = fgint(depth))
        )
        if not result.isa(String) and not isinstance(result.value(), str):
            raise FugaError("str must return a primitive string")
        return result

    def __repr__(self):
        return self.str().value()

class fgthunk(fgobj):
    def __init__(self, code, env):
        code.need()
        self._strict = False
        self._code = code
        self._env  = env

    def _transfer(self, other):
        if self._strict:
            raise FugaError("thunk transfer: can't transfer twice")
        other.need()
        self._slots = other._slots
        self._proto = other._proto
        self._value = other._proto
        self._length = other._length
        self._strict = True
        del self._code
        del self._env

    def need(self):
        if self._strict is None:
            raise FugaError("thunk need: cyclic dependency")
        if not self._strict:
            self._strict = None
            self._transfer(self._code.eval(self._env))
    
    def thunkSlots(self, scope):
        if not self._strict:
            scope  = self._env.clone()
            result = fgobj(Object)
            for k in self._code._slots:
                thunk = fgthunk(self.rawGet(k), scope)
                scope [k] = thunk
                result[k] = thunk
            self._transfer(result)
    
    def code(self):
        if self._strict:
            raise FugaError("thunk code: thunk was already evaluated")
        return self._code

    def scope(self):
        if self._strict:
            raise FugaError("thunk scope: thunk was already evaluated")
        return self._scope

############################################
## Primitive ###############################
############################################

Object = fgobj(None)

Int    = Object['Int']    = Object.clone()
String = Object['String'] = Object.clone()
Symbol = Object['Symbol'] = Object.clone()
Msg    = Object['Msg']    = Object.clone()
Method = Object['Method'] = Object.clone()
Quote  = Object['Quote']  = Object.clone()
Expr   = Object['Expr']   = Object.clone()

# Primitives
def fgint(val):
    assert isinstance(val, int)
    return Int.clone(val)

def fgstr(val):
    assert isinstance(val, str)
    return String.clone(val)

def fgsym(val, _memo={}):
    assert isinstance(val, str)
    if val in _memo:
        return _memo[val]
    symbol = Symbol.clone(val)
    _memo[val] = symbol
    return symbol

def fgmethod(val):
    assert isinstance(val, collections.Callable)
    return Method.clone(val)

def fgmsg(name, *vargs, **kwargs):
    assert isinstance(name, str)
    return Msg.clone(name, *vargs, **kwargs)

def fgquote(val):
    return Quote.clone(value=val)

def fgexpr(*vargs):
    expr = Expr.clone(*vargs)
    if not len(expr):
        raise TypeError("Expr must have at least one indexed slot.")
    return expr

###########################################
## Parser #################################
###########################################

class _parser(PEG):
    m_start = 'Module'

    # Hierarchical
    m_Module       = 'Spacing Block EndOfFile'
    m_Block        = 'Slot? (SEPARATOR+ Slot)* SEPARATOR*'
    m_Slot         = 'Slot_EQUALS / Slot_BECOMES / Expr'
    m_Slot_EQUALS  = 'Name+ EQUALS Expr'
    m_Slot_BECOMES = 'Name+ Object? BECOMES Expr'
    m_Expr         = 'Part' # a simplification for now
    m_Part         = 'PrefixOp* Root Msg*'
    m_Root         = 'PExpr / Object / Int / String / Msg / Symbol'
    m_PExpr        = 'LBRACKET Expr RBRACKET'
    m_Object       = 'LPAREN Block RPAREN'
    m_Msg          = 'Name Object? / DeceptiveOp Object?'

    def Module(self,v): return v[1]
    def Block(self,v):
        result = Object.clone()
        slots  = [v[0]] if v[0] is not None else []
        slots.extend(w[1] for w in v[1])
        for slot in slots:
            if isinstance(slot, tuple):
                result.set(slot[0], slot[1])
            else:
                result.append(slot)
        return result

    def Slot_EQUALS(self,v):
        if len(v[0]) == 1:
            return v[0][-1], v[2]
        msg = fgmsg('set', fgsym(v[0][-1]), v[2])
        msgs = list(map(fgmsg, v[0][:-1]))
        msgs.append(msg)
        return fgexpr(*msgs)

    def Slot_BECOMES(self, v):
        args = Object.clone() if v[1] is None else v[1]
        method = fgmsg('method', args, v[3])
        w = [v[0], '=', method]
        return self.Slot_EQUALS(w)

    def Part(self, v):
        if v[0]:
            return self.Part((
                v[0][:-1],
                fgmsg(v[0][-1], v[1]),
                v[2]
            ))
        if len(v[2]):
            return fgexpr(v[1], *v[2])
        return v[1]

    def Msg(self, v):
        result = fgmsg(v[0])
        if v[1] is not None:
            result._slots = v[1]._slots
            result._length = v[1]._length
        return result

    def PExpr(self, v): return v[1]
    def Object(self, v): return v[1]


    # Lexical
    m_LBRACKET     = '"[" LineSpacing'
    m_RBRACKET     = 'LineSpacing "]" Spacing'
    m_LPAREN       = '"(" Spacing'
    m_RPAREN       = '")" Spacing'
    m_SEPARATOR    = '[,\r\n] LineSpacing'
    m_EQUALS       = '"=" !RawOp LineSpacing'
    m_BECOMES      = '"=>" !RawOp LineSpacing'
    m_SYMPREFIX    = '":" !RawOp Spacing'
    
    m_Op           = '!EQUALS !BECOMES !SYMPREFIX RawOp LineSpacing'
    m_RawOp        = r'[`~!@$%^&*\-+=|:;.<>/?]+'
    m_PrefixOp     = '!DeceptiveOp Op'
    m_DeceptiveOp  = '&(RawOp "(") Op'
    
    m_Symbol       = 'SYMPREFIX RawName Spacing'
    m_Name         = '!Int RawName Spacing'
    m_Int          = 'RawInt !RawName Spacing'
    m_RawName      = r'[a-zA-Z0-9_][a-zA-Z0-9_?!]* / "\\" RawOp'

    def Op(self,v): return ''.join(v[3])
    def PrefixOp(self, v): return v[1]
    def DeceptiveOp(self, v): return v[1]
    def Symbol(self,v): return fgsym(v[1])
    def Name(self,v): return v[1]
    def Int(self,v): return v[0]
    def RawName(self,v): return v[0] + ''.join(v[1])

    m_RawInt       = 'BinInt / HexInt / DecInt'
    m_BinInt       = '"0b" [01]+'
    m_DecInt       = '"0d"? [0-9]+'
    m_HexInt       = '"0x" [0-9a-fA-F]+'

    def BinInt(self,v):
        total = 0
        digits = {'0':0, '1':1}
        base   = 2
        for c in v[1]:
            total *= base
            total += digits[c]
        return fgint(total)

    def HexInt(self,v):
        total = 0
        digits = {'0':0, '1':1, '2':2, '3':3, '4':4,
                  '5':5, '6':6, '7':7, '8':8, '9':9,
                  'a':10, 'b':11, 'c':12, 'd':13, 'e':14, 'f':15,
                  'A':10, 'B':11, 'C':12, 'D':13, 'E':14, 'F':15}
        base = 16
        for c in v[1]:
            total *= base
            total += digits[c]
        return fgint(total)

    def DecInt(self,v):
        total = 0
        digits = {'0':0, '1':1, '2':2, '3':3, '4':4,
                  '5':5, '6':6, '7':7, '8':8, '9':9}
        base = 10
        for c in v[1]:
            total *= base
            total += digits[c]
        return fgint(total)

    m_String       = 'StringFrag+'
    m_StringFrag   = r'"\"" (![\n\"] Char)* "\"" Spacing'
    m_Char         = r'"\\"? .'

    def String(self, v):
        return fgstr(''.join(v))

    def StringFrag(self, v):
        return ''.join([w[1] for w in v[1]])
        
    def Char(self, v):
        repls = {'n': '\n', 'r': '\r', 't': '\t'}
        if v[0] is not None:
            return repls.get(v[1], v[1])
        else:
            return v[1]

    m_Comment      = '"#" (!"\n" .)*'
    m_EscapedLine  = r'"\\" RawSpacing* [\r\n]+'
    m_RawSpacing   = '[ \t]+ / Comment'
    m_Spacing      = '(EscapedLine / RawSpacing)*'
    m_LineSpacing  = '(EscapedLine / [ \t\r\n]+ / Comment)*'
    m_EndOfFile    = '!.'

parser = _parser()

def parse(code):
    r"""
    >>> parse('')
    ()
    >>> parse('soprano')
    (soprano)
    >>> parse('soprano=alto')
    (soprano=alto)
    >>> parse('soprano alto=tenor')
    (soprano set(:alto, tenor))
    >>> parse('soprano => alto')
    (soprano=method((), alto))
    >>> parse('soprano alto => tenor')
    (soprano set(:alto, method((), tenor)))
    >>> parse('soprano alto tenor bass')
    (soprano alto tenor bass)
    >>> parse('+soprano')
    (+(soprano))
    >>> parse('- -soprano')
    (-(-(soprano)))
    >>> parse('+soprano alto')
    (+(soprano) alto)
    >>> parse('()')
    (())
    >>> parse('(soprano)')
    ((soprano))
    >>> parse('(soprano, alto\ntenor\tbass)')
    ((soprano, alto, tenor bass))
    >>> parse('[soprano]')
    (soprano)
    >>> parse('+[soprano alto]')
    (+(soprano alto))
    >>> parse('foo?!?!')
    (foo?!?!)
    >>> parse('soprano(alto)')
    (soprano(alto))
    >>> parse('soprano(alto, tenor(bass))')
    (soprano(alto, tenor(bass)))
    >>> parse('+(soprano)')
    (+(soprano))
    >>> parse('10')
    (10)
    >>> parse('0xFF')
    (255)
    >>> parse('0b10')
    (2)
    >>> parse('"hello"')
    ("hello")
    >>> parse(r'"world\""')
    ("world\"")
    >>> parse(r'"\n\t\r\\\""')
    ("\n\t\r\\\"")
    >>> parse('"soprano" "alto"')
    ("sopranoalto")
    >>> parse('foo #bar\nbaz')
    (foo, baz)
    >>> parse('foo \\#bar\nbaz')
    (foo baz)
    >>> parse(':hello')
    (:hello)
    """
    return parser.parse(code)

###########################################
## Important methods ######################
###########################################

# handy decorator
def setAt(obj, name):
    def decorator(val):
        obj[name] = fgmethod(val)
        return val
    return decorator

# Stringifying!
@setAt(Object, 'str')
def Object_str(self, args):
    if len(args):
        raise FugaError("Object str: expected no arguments")
    elif self is Object:
        return fgstr('Object')
    elif 'depth' in args:
        if args['depth'].isa(Int):
            return self.strSlots(args['depth'].value())
        else:
            raise FugaError("Object str: depth must be an Int")
    else:
        return self.strSlots()

@setAt(Int, 'str')
def Int_str(self, args):
    if len(args):
        raise FugaError("Int str: expected no arguments")
    return fgstr(str(self.value()))

@setAt(String, 'str')
def String_str(self, args):
    if len(args):
        raise FugaError("String str: expected no arguments")
    reps = {'\n': '\\n', '\r': '\\r', '\t': '\\t',
             '"': '\\"', '\\': '\\\\'}
    result = ''.join(reps.get(c,c) for c in self.value())
    return fgstr('"%s"' % result)

@setAt(Symbol, 'str')
def Symbol_str(self, args):
    if len(args):
        raise FugaError("Symbol str: expected no arguments")
    return fgstr(':' + self.value())

Method['str'] = fgstr('method(...)')

@setAt(Quote, 'str')
def Quote_str(self, args):
    if len(args):
        raise FugaError("Quote str: expected no arguments")
    return fgstr("'[" + asdfasdfasdf + "]")

@setAt(Expr, 'str')
def Expr_str(self, args):
    if len(args):
        raise FugaError("Expr str: expected no arguments")
    if 'depth' in args:
        if args['depth'].isa(Int):
            depth = args['depth'].value()
        else:
            raise FugaError("Expr str: depth must be an Int")
    else:
        depth = 4
    
    slots = []
    for slot in self:
        slots.append(slot.str(depth-1).value())
    return fgstr(' '.join(slots))

@setAt(Msg, 'str')
def Msg_str(self, args):
    if len(args):
        raise FugaError("Msg str: expected no arguments")
    if 'depth' in args:
        if args['depth'].isa(Int):
            depth = args['depth'].value()
        else:
            raise FugaError("Expr str: depth must be an Int")
    else:
        depth = 4

    if list(self._slots.keys()):
        return fgstr(self.value() + self.strSlots(depth).value())
    else:
        return fgstr(self.value())

#### TESTING FRAMEWORK ####
if __name__ == '__main__':
    testmod()
