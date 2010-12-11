
from peg     import PEG
from doctest import testmod

class FugaError(Exception):
    pass

class fgobj(object):
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
        ... except FugaError, e: print e
        Object proto: Object has no prototype
        """
        self.need()
        if self._proto is not None:
            return self._proto
        else:
            raise FugaError, "Object proto: Object has no prototype"

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
            raise TypeError, "expected only int or str for slot name"
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
        ... except FugaError, e: print e
        Object rawGet: object has no slot soprano
        >>>
        >>> tenor.rawGet(0) is alto
        True
        >>> try: bass.rawGet(0)
        ... except FugaError, e: print e
        Object rawGet: object has no slot 0
        """
        if self.rawHas(name):
            return self._slots[name]
        raise FugaError, "Object rawGet: object has no slot %s" % name

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
        ... except FugaError, e: print e
        Object get: object has no slot 0
        """
        if self.rawHas(name):
            return self._slots[name]
        elif self._proto is not None and isinstance(name, str):
            return self._proto.get(name)
        else:
            raise FugaError, "Object get: object has no slot %s" % name

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
            raise TypeError, "expected only int or str for slot name"
        if not isinstance(value, fgobj):
            raise TypeError, "expected a fuga object for slot value"
        self.need()
        if name in self._slots:
            raise FugaError, "Object set: object already has slot %s"%name
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
        return self._proto and (
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
            raise FugaError, "Method activate: non-primitive methods" \
                                               + " not yet supported"
        else:
            args = args.thunkSlots()
            if args._slots.keys():
                raise FugaError, "Object activate: expected no arguments"

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
        if depth == 0:
            return '(...)'
        slots = []
        for k in self._slots:
            if (isinstance(k, str) and k[0] != '_') or k > self.length():
                slots.append('%s = %s' % (k, self._slots[k].str(depth-1)))
        for slot in self:
            slots.append(k.str(depth-1))
        return '(%s)' % ', '.join(slots)
    
    def str(self, depth=4):
        if self is Object:
            return 'Object'

    def __repr__(self):
        return self.str()

class fgthunk(fgobj):
    def __init__(self, code, env):
        code.need()
        self._strict = False
        self._code = code
        self._env  = env

    def _transfer(self, other):
        if self._strict:
            raise FugaError, "thunk transfer: can't transfer twice"
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
            raise FugaError, "thunk need: cyclic dependency"
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
            raise FugaError, "thunk code: thunk was already evaluated"
        return self._code

    def scope(self):
        if self._strict:
            raise FugaError, "thunk scope: thunk was already evaluated"
        return self._scope

############################################
## IMPORTANT GLOBALS #######################
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
    if val in memo:
        return memo[val]
    symbol = Symbol.clone(val)
    memo[val] = symbol
    return symbol

def fgmethod(val):
    assert callable(val)
    return Method.clone(val)

def fgmsg(name, *vargs, **kwargs):
    assert isinstance(name, str)
    return Method.clone(name, *vargs, **kwargs)

def fgquote(val):
    return Quote.clone(value=val)

def fgexpr(*vargs):
    return Expr.clone(*vargs)

#### TESTING FRAMEWORK ####
if __name__ == '__main__':
    testmod()

