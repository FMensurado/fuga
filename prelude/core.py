#!/usr/bin/env python3

import collections

class FugaError(Exception):
    pass

class GuardError(FugaError):
    pass

STRDEPTH = 7

_objid = 0
def _newid():
    global _objid
    _objid += 1
    return _objid

class fgobj:
    def __init__(self, proto, *vargs, **kwargs):
        self._proto = proto
        self._slots = {}
        self._length = 0
        self._value  = None
        self._id = _newid()
        
        if vargs and not isinstance(vargs[0], fgobj):
            self._value = vargs[0]
            vargs = vargs[1:]

        for i, varg in enumerate(vargs):
            self.set(i, varg)
        for k in kwargs:
            self.set(k, kwargs[k])

    def id(self):
        self.need()
        return self._id

    def is_(self, other):
        return self.id() == other.id()

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

    def update(self, other):
        """
        Copy all named slots from other into self.

        >>> obj = Object.clone(fgint(10))
        >>> obj
        (10)
        >>> obj.update(Object.clone(Object, a=Object))
        >>> obj
        (10, a=Object)
        """
        for name in other._slots:
            if isinstance(name, str):
                self[name] = other[name]

    def extend(self, other):
        """
        Append all indexed slots from other into self.
        
        >>> obj = Object.clone(fgint(10))
        >>> obj
        (10)
        >>> obj.extend(Object.clone(Object, a=Object))
        >>> obj
        (10, Object)
        >>> obj = Object.clone()
        >>> obj.extend(Object.clone(fgint(10), fgint(20), fgint(30)))
        >>> obj
        (10, 20, 30)
        """
        for slot in other:
            self.append(slot)


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


    def handle(self):
        if self.has('name') and self['name'].isPrimitive(str):
            if self.rawHas('name'):
                return self['name'].value()
            elif self['name'].value() == 'Object':
                return 'object'
            else:
                return self['name'].value() + ' object'
        else:
            return 'object'

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
        raise FugaError("Object rawGet: %s has no slot %s" %
                    (self.handle(), name))

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
            raise FugaError("Object get: %s has no slot %s" %
                    (self.handle(), name))

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
        #if name in self._slots:
        #    raise FugaError("Object set: %s already has slot %s" %
        #            (self._handle(), name))
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
            self._proto.is_(proto)  or
            self._proto.isa(proto)
        )

    def clone(self, *vargs, **kwargs):
        return fgobj(self, *vargs, **kwargs)

    def eval(self, receiver, scope=None, reflect=False, bleed=False):
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
                receiver = slot.eval(receiver, scope)
            return receiver
        elif self.isa(Msg):
            fn = receiver.get(self.value())
            args = self.args(scope)
            return fn.activate(receiver, args)
        else:
            return self.evalSlots(scope, reflect, bleed)


    def slots(self):
        self.need()
        slots = Object.clone()
        slots._slots = self._slots
        slots._length = self._length
        return slots

    def args(self, scope):
        return fgthunk(self.slots(), scope)

    def toArg(self, scope, name):
        """
        >>> fgint(10).toArg(Object, 0)
        (10)
        >>> fgint(10).toArg(Object, 10)
        (10)
        >>> fgint(10).toArg(Object, 'a')
        (a=10)
        >>> fgint(10).toArg(Object, 'b')
        (b=10)
        """
        if self.isa(Msg) and self.value() == '~' and len(self) == 1:
            thunk = self[0].eval(scope)
            if not thunk.isa(Thunk):
                raise FugaError("can't use ~ on non-Thunks!")
            return thunk['code'].toArg(thunk['scope'], name)
        if self.isa(Msg) and self.value() == '*' and len(self) == 1:
            return self[0].eval(scope)
        else:
            arg = Object.clone()
            if isinstance(name, int):
                arg.append(fgthunk(self, scope))
            else:
                arg[name] = fgthunk(self, scope)
            return arg

    def thunkSlots(self):
        pass

    def splitThunk(self):
        return self

    def activate(self, receiver, args):
        if self.isa(Method):
            if self.value():
                return self.value()(receiver, args)
            i = 0
            while i < len(self):
                try:
                    formals = self[i]
                    body    = self[i+1]
                    i += 2

                    params = formals.match(args)
                    env = self['scope'].clone()
                    env['self'] = receiver
                    env.update(params)
                    return body.eval(env)
                except GuardError:
                    continue
            else:
                raise FugaError("Method activate: No matching pattern.")

        elif len(args):
            raise FugaError("Object activate: non-method expects"
                                                + " no arguments")
        return self


    def isthunk(self):
        return False

    def match(self, candidate):
        if 'match' in self and self['match'].isa(Method):
            return self['match'].activate(self, Object.clone(candidate))
        raise FugaError("match: no callable match method for object")

    def matchSlots(self, candidate):
        self.need()
        for k in self._slots:
            if 'matchByThunk?' in self[k]:
                if self[k]['matchByThunk?'].activate(
                                               self[k],
                                               Object.clone()
                                           ).is_(fgtrue):
                    candidate = candidate.splitThunk()
                    break

        if len(self) != len(candidate):
            raise GuardError("Wrong number of arguments.")
        captures = Object.clone()
        for k in self._slots:
            if k in candidate:
                captures.update(self[k].match(candidate[k]))
            else:
                raise GuardError()
        return captures

    def evalSlots(self, scope, reflect=False, bleed=False):
        result = fgthunk(self, scope).splitThunk(reflect, bleed)
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

    def strSlots(self, depth=STRDEPTH):
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

    def str(self, depth=STRDEPTH):      
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

    def isthunk(self):
        return not self._strict

    def _transfer(self, other):
        if self._strict:
            raise FugaError("thunk transfer: can't transfer twice")
        other.need()
        self._slots = other._slots
        self._proto = other._proto
        self._value = other._value
        self._length = other._length
        self._strict = True
        self._id     = other._id
        del self._code
        del self._env

    def need(self):
        if self._strict is None:
            raise FugaError("thunk need: cyclic dependency")
        if not self._strict:
            try:
                self._strict = None
                value = self._code.eval(self._env)
            except FugaError as e:
                self._strict = False
                raise e
            self._transfer(value)
    
    def thunkSlots(self, reflect=False, bleed=False):
        if not self._strict:
            self._transfer(self.splitThunk(reflect, bleed))

    def splitThunk(self, reflect=False, bleed=False):
        """
        >>> len(fgthunk(Object.clone(Int, Int, Int), Object).splitThunk())
        3
        """
        if self._strict:
            return self

        result = Object.clone()
        
        scope = self._env
        if reflect and not bleed:
            scope = scope.clone()
        
        self._code.need()
        
        for k in self._code._slots:
            arg = self._code[k].toArg(scope, k)
            if reflect:
                scope.update(arg)
            result.update(arg)
            result.extend(arg)
        
        return result

    def code(self):
        if self._strict:
            raise FugaError("thunk code: thunk was already evaluated")
        return self._code

    def scope(self):
        if self._strict:
            raise FugaError("thunk scope: thunk was already evaluated")
        return self._env

############################################
## Primitive ###############################
############################################

Object = fgobj(None)

Number = Object.clone()
Int    = Number.clone()

String = Object.clone()
Symbol = Object.clone()
Msg    = Object.clone()
Method = Object.clone()
Expr   = Object.clone()
Bool   = Object.clone()
void   = Object.clone()

fgtrue  = Bool.clone(True)
fgfalse = Bool.clone(False)

Thunk = Object.clone()

# Primitives / Basic Objects
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

def fgfgthunk(code, scope):
    return Thunk.clone(code=code, scope=scope)

def fgname(name):
    if isinstance(name, str):
        return fgsym(name)
    else:
        return fgint(name)

###########################################
## Important methods ######################
###########################################

# handy decorators
def setmethod(obj, name, nargs=None, passargs=False):
    if nargs is None:
        def decorator(fn):
            obj[name] = fgmethod(fn)
            return fn
        return decorator
    elif not isinstance(nargs, int):
        raise TypeError("nargs must be an int")
    else:
        if obj.rawHas('name') and obj['name'].isPrimitive(str):
            prefix = obj['name'].value() + ' ' + name
        else:
            prefix = name

        if nargs == 1:
            prefix += ': expected 1 argument, got '
        else:
            prefix += ': expected %d arguments, got ' % nargs

        def decorator(fn):
            def newfn(self, args):
                if len(args) != nargs:
                    raise FugaError(prefix + str(len(args)))
                vargs = []
                for slot in args:
                    vargs.append(slot)
                if passargs:
                    vargs.append(args)
                return fn(self, *vargs)
            obj[name] = fgmethod(newfn)
            return obj[name]
        return decorator

def setstr(obj, needdepth=False):
    name = 'str'
    if needdepth:
        def decorator(fn):
            @setmethod(obj, 'str', 0, True)
            def newfn(self, args):
                if self.rawHas('name') and self['name'].isPrimitive(str):
                    return self['name']
                if 'depth' in args:
                    if (args['depth'].isPrimitive(int) and
                        args['depth'].isa(Int)):
                        depth = args['depth'].value()
                    else:
                        raise FugaError('str: depth must be a primitive'
                                                                   'int')
                else:
                    depth = STRDEPTH
                return fn(self, depth)
            return newfn
        return decorator
    else:
        def decorator(fn):
            @setmethod(obj, 'str', 0)
            def newfn(self):
                if self.rawHas('name') and self['name'].isPrimitive(str):
                    return self['name']
                return fn(self)
            return newfn
        return decorator

# Name
Object['name'] = fgstr('Object')
Int   ['name'] = fgstr('Int')
String['name'] = fgstr('String')
Symbol['name'] = fgstr('Symbol')
Method['name'] = fgstr('Method')
Msg   ['name'] = fgstr('Msg')
Expr  ['name'] = fgstr('Expr')
Bool  ['name'] = fgstr('Bool')
void  ['name'] = fgstr('void')

fgtrue ['name'] = fgstr('true')
fgfalse['name'] = fgstr('false')

Thunk ['name'] = fgstr('Thunk')

# Stringifying!
@setstr(Object, True)
def Object_str(self, depth):
    return self.strSlots(depth)

@setmethod(Object, 'strSlots', 0, True)
def Object_strSlots(self, args):
    if 'depth' in args:
        if (args['depth'].isPrimitive(int) and
            args['depth'].isa(Int)):
            depth = args['depth'].value()
        else:
            raise FugaError('strSlots: depth must be a primitive int')
    else:
         depth = STRDEPTH
    return self.strSlots(depth)

@setstr(String)
def String_str(self):
    reps = {'\n': '\\n', '\r': '\\r', '\t': '\\t',
             '"': '\\"', '\\': '\\\\'}
    result = ''.join(reps.get(c,c) for c in self.value())
    return fgstr('"%s"' % result)

@setstr(Int)
def Int_str(self): return fgstr(str(self.value()))

@setstr(Symbol)
def Symbol_str(self): return fgstr(':' + self.value())

@setstr(Method)
def Method_str(self): return fgstr('method(...)')


@setstr(Expr, True)
def Expr_str(self, depth):
    slots = []
    for slot in self:
        slots.append(slot.str(depth-1).value())
    return fgstr(' '.join(slots))

@setstr(Msg, True)
def Msg_str(self, depth):
    if list(self._slots.keys()):
        return fgstr(str(self.value()) + self.strSlots(depth).value())
    else:
        return fgstr(str(self.value()))
 
@setstr(Thunk)
def Thunk_str(self):
    return fgstr('thunk(...)')

## Matching

@setmethod(Object, 'match', 1)
def Object_match(self, candidate):
    return self.matchSlots(candidate)

@setmethod(Msg, 'matchByThunk?', 0)
def Msg_matchByThunk(self):
    if self.value() == '~' and len(self) == 1:
        return fgtrue
    return fgfalse


@setmethod(Msg, 'match', 1)
def Msg_match(self, candidate):
    captures = Object.clone()
    if self.value() == '~' and len(self) == 1 and self[0].isa(Msg):
        captures[self[0].value()] = fgfgthunk(
            candidate.code(),
            candidate.scope()
        )
    elif len(self):
        raise FugaError("this doesn't make sense")
    else:
        captures[self.value()] = candidate
    return captures

@setmethod(Int, 'match', 1)
def Int_match(self, candidate):
    if (candidate.isa(Int) and candidate.isPrimitive(int)
                           and self.value() == candidate.value()):
        return Object.clone()
    raise GuardError("Int doesn't match.")

@setmethod(String, 'match', 1)
def Int_match(self, candidate):
    if (candidate.isa(String) and candidate.isPrimitive(str)
                              and self.value() == candidate.value()):
        return Object.clone()
    raise GuardError("String doesn't match.")

@setmethod(Symbol, 'match', 1)
def Int_match(self, candidate):
    if (candidate.isa(Symbol) and candidate.isPrimitive(str)
                              and self.value() == candidate.value()):
        return Object.clone()
    raise GuardError("String doesn't match.")

## Slot manipulation

def slotmanip(method, wrap=None):
    if wrap is None:
        wrap = lambda x: x
    @setmethod(Object, method, 1)
    def fn(self, name):
        if name.isPrimitive(int) or name.isPrimitive(str):
            return wrap(getattr(self, method)(name.value()))
        raise FugaError("Object %s: expected primitive Int, String, "
                        "Symbol, or Msg." % name)


slotmanip('get')
slotmanip('rawGet')
slotmanip('has', fgbool)
slotmanip('rawHas', fgbool)

@setmethod(Object, 'set', 2)
def Object_set(self, name, value):
    if name.isPrimitive(int) or name.isPrimitive(str):
        self.set(name.value(), value)
        return self
    raise FugaError("Object set: name must be an Int, String, "
                                              "Symbol, or Msg.")

@setmethod(Object, 'append', 1)
def Object_append(self, value):
    self.append(value)
    return self

# Other basic methods.

@setmethod(Object, 'clone')
def Object_clone(self, args):
    args.need()
    result = self.clone()
    result._slots = args._slots
    result._length = args._length
    return result

@setmethod(Object, 'proto', 0)
def Object_proto(self):
    return self.proto()

@setmethod(Object, 'len', 0)
def Object_len(self):
    return fgint(self.length())

@setmethod(Object, 'isa', 1)
def Object_isa(self, proto):
    return fgbool(self.isa(proto))

# Testing

if __name__ == '__main__':
    import doctest
    doctest.testmod()
