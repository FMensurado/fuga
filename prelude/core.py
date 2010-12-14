#!/usr/bin/env python3

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
        ... except FugaError as e: print(e)
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
        ... except FugaError as e: print(e)
        Object rawGet: object has no slot soprano
        >>>
        >>> tenor.rawGet(0) is alto
        True
        >>> try: bass.rawGet(0)
        ... except FugaError as e: print(e)
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
        ... except FugaError as e: print(e)
        Object get: object has no slot 0
        """
        if self.rawHas(name):
            return self._slots[name]
        elif (self._proto is not None and isinstance(name, str)
                                      and self._proto.has(name)):
            return self._proto.get(name)
        else:    
            if self['name'].isPrimitive(str):
                raise FugaError("Object get: %s has no slot %s" %
                        (self['name'].value(), name))
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
            if self['name'].isPrimitive(str):
                raise FugaError("Object set: %s already has slot %s" %
                        (self['name'].value(), name))
            else:
                raise FugaError("Object set: object already has slot %s"
                        % name)
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
        """
        >>> Int.isa(Int)
        False
        >>> fgint(10).isa(Int)
        True
        >>> Object.isa(Object)
        False
        >>> Int.isa(Object)
        True
        >>> fgint(10).isa(Object)
        True
        """
        self.need()
        return (self._proto is not None) and (
            self._proto is proto  or
            self._proto.isa(proto)
        )

    def clone(self, *vargs, **kwargs):
        return fgobj(self, *vargs, **kwargs)

    def eval(self, receiver, scope=None):
        """
        >>> fgint(10).eval(Object)
        10
        >>> fgstr("Hello, World!").eval(Object)
        "Hello, World!"
        >>> fgsym("doremi").eval(Object)
        :doremi
        >>> Object.clone(fgint(10), fgint(20)).eval(Object)
        (10, 20)
        >>> fgexpr(fgint(10), fgint(20)).eval(Object)
        20
        >>> fgmsg("a").eval(Object.clone(a=fgint(10)))
        10
        >>> fgmsg("str").eval(Object)
        "Object"
        >>> fgexpr(fgmsg("a"),fgmsg("str")).eval(Object.clone(a=fgint(10)))
        "10"

        """
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
            return fn.activate(receiver, fgthunk(args, scope))
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

    def needSlots(self):
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
    
    def isPrimitive(self, type=None):        
        if type is not None:
            return isinstance(self.value(), type)
        else:
            return self.value() is not None

    def str(self, depth=4):      
        assert isinstance(depth, int)
        if self.rawHas('name') and self['name'].isPrimitive(str):
            return self['name']
        result = self['str'].activate(self,
            Object.clone(depth = fgint(depth))
        )
        if not result.isa(String) and not result.isinstance(str):
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
        self._value = other._value
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
    
    def thunkSlots(self):
        if not self._strict:
            self._strict = True
            self._slots  = {}
            self._proto  = Object
            self._value  = None
            self._length = 0
            scope = self._env.clone()
            for k in self._code._slots:
                thunk = fgthunk(self._code.rawGet(k), scope)
                scope[k] = thunk
                self [k] = thunk
            del self._env
            del self._code
    
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
Object['Object'] = Object.clone()

Int    = Object['Int']    = Object.clone()
String = Object['String'] = Object.clone()
Symbol = Object['Symbol'] = Object.clone()
Msg    = Object['Msg']    = Object.clone()
Method = Object['Method'] = Object.clone()
Quote  = Object['Quote']  = Object.clone()
Expr   = Object['Expr']   = Object.clone()
Bool   = Object['Bool']   = Object.clone()

fgtrue  = Object['true']   = Bool.clone(True)
fgfalse = Object['false']  = Bool.clone(False)

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
    if isinstance(name, fgobj):
        name = name.value()
    assert isinstance(name, str) or isinstance(name, int)
    return Msg.clone(name, *vargs, **kwargs)

def fgquote(val):
    return Quote.clone(value=val)

def fgexpr(*vargs):
    expr = Expr.clone(*vargs)
    if not len(expr):
        raise TypeError("Expr must have at least one indexed slot.")
    return expr

def fgbool(m):
    if m:
        return fgtrue
    else:
        return fgfalse

###########################################
## Important methods ######################
###########################################

# handy decorator
def setAt(obj, name):
    def decorator(val):
        obj[name] = fgmethod(val)
        return val
    return decorator

# Name
Object['name'] = fgstr('Object')
Int   ['name'] = fgstr('Int')
String['name'] = fgstr('String')
Symbol['name'] = fgstr('Symbol')
Quote ['name'] = fgstr('Quote')
Method['name'] = fgstr('Method')
Msg   ['name'] = fgstr('Msg')
Expr  ['name'] = fgstr('Expr')
Bool  ['name'] = fgstr('Bool')

fgtrue ['name'] = fgstr('true')
fgfalse['name'] = fgstr('false')

# Stringifying!
@setAt(Object, 'str')
def Object_str(self, args):
    if len(args):
        raise FugaError("Object str: expected no arguments")
    elif self.rawHas('name'):
        return self['name']
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
    elif self is Int:
        return fgstr('Int')
    return fgstr(str(self.value()))

@setAt(String, 'str')
def String_str(self, args):
    if len(args):
        raise FugaError("String str: expected no arguments")
    elif self is String:
        return fgstr('String')
    reps = {'\n': '\\n', '\r': '\\r', '\t': '\\t',
             '"': '\\"', '\\': '\\\\'}
    result = ''.join(reps.get(c,c) for c in self.value())
    return fgstr('"%s"' % result)

@setAt(Symbol, 'str')
def Symbol_str(self, args):
    if len(args):
        raise FugaError("Symbol str: expected no arguments")
    elif self is Symbol:
        return fgstr('Symbol')
    return fgstr(':' + self.value())

Method['str'] = fgstr('method(...)')

@setAt(Quote, 'str')
def Quote_str(self, args):
    if len(args):
        raise FugaError("Quote str: expected no arguments")
    elif self is Quote:
        return fgstr('Quote')
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
        return fgstr(str(self.value()) + self.strSlots(depth).value())
    else:
        return fgstr(str(self.value()))

## Slot manipulation

def slotmanip(method, wrap=None):
    if wrap is None:
        wrap = lambda x: x
    @setAt(Object, method)
    def fn(self, args):
        if len(args) != 1:
            raise FugaError("Object %s: expected 1 argument, got %d" %
                (method, len(args))
            )
        name = args[0]
        if name.isPrimitive(int) or name.isPrimitive(str):
            return getattr(self, method)(name.value())
        raise FugaError("Object %s: expected primitive Int, String, "
                        "Symbol, or Msg." % name)


slotmanip('get')
slotmanip('rawGet')
slotmanip('has', fgbool)
slotmanip('rawHas', fgbool)

@setAt(Object, 'set')
def Object_set(self, args):
    if len(args) != 2:
        raise FugaError("Object set: expected 2 arguments, got %d" %
            len(args))
    name  = args[0]
    value = args[1]
    if name.isPrimitive(int) or name.isPrimitive(str):
        self.set(name.value(), value)
        return self
    raise FugaError("Object set: name must be an Int, String, "
                                              "Symbol, or Msg.")

@setAt(Object, 'append')
def Object_append(self, args):
    if len(args) != 1:
        raise FugaError("Object append: expected 1 argument, got %d" %
            len(args))
    self.append(args[0])
    return self

if __name__ == '__main__':
    import doctest
    doctest.testmod()
