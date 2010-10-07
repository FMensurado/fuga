
class FugaError(Exception):
    pass

class fgobject(object):
    def __init__(self, proto, *slots):
        if slots and not isinstance(slots[0], tuple):
            self._value = slots[0]
            slots = slots[1:]
        else:
            self._value = None
        self.proto = proto
        self.slots = list(slots)
        self.byName = {}
        for i,(k,v) in enumerate(slots):
            if k is not None:
                self.byName[k] = i

    def value(self):
        if self._value is not None:
            return self._value
        elif self.proto:
            return self.proto.value()
        else:
            return None

    def clone(self, *slots):
        return fgobject(self, *slots)

    def set(self, name, value):
        if isinstance(name, str) or name is None:
            if name in self.byName:
                raise FugaError, "Can't have two slots with name %s" % name
            if name is not None:
                self.byName[name] = len(self.slots)
            self.slots.append((name, value))
        else:
            raise TypeError, "Can't set slot by type %s." % type(name)

    def rawHas(self, name):
        if isinstance(name, str):
            return name in self.byName
        elif isinstance(name, int):
            return name  >= 0 and name < len(self.slots)
        else:
            raise TypeError, "Can't get slot by type %s." % type(name)

    def has(self, name):
        return self.rawHas(name) or (self.proto and self.proto.has(name))

    def rawGet(self, name):
        if not self.rawHas(name):
            raise FugaError, "No slot named %s." % name
        elif isinstance(name,str):
            return self.slots[self.byName[name]][1]
        elif isinstance(name,int):
            return self.slots[name][1]

    def get(self, name):
        if self.rawHas(name):
            return self.rawGet(name)
        elif self.proto and self.proto.has(name):
            return self.proto.get(name)
        else:
            raise FugaError, "No slot named %s." % name

    def isA(self, proto):
        return self.proto and (self.proto is proto
                           or  self.proto.isA(proto))

    def eval(self, obj, env=None, callenv=None):
        """
        >>> Object.eval(fgint(10))
        10
        >>> Object.eval(fgreal(3.25))
        3.25
        >>> Object.eval(fgstr('Hello, world!'))
        "Hello, world!"
        >>> Object.clone(('foo', fgtrue)).eval(fgmsg('foo'))
        true
        >>> Object.clone(('foo', fgmethod(lambda x,y,z: 100))).eval(fgmsg('foo'))
        100
        >>> Object.clone(('foo', fgmethod(lambda x,y,z: y))).eval(fgmsg('foo')).slots
        []
        >>> Object.clone(('foo', fgmethod(lambda x,y,z: y))).eval(fgmsg('foo', (None, None))).slots
        [(None, None)]
        >>> Object.clone(('foo', Object.clone(('bar', fgtrue)))).eval(fglist(fgmsg('foo'), fgmsg('bar')))
        true
        """
        if not env:
            env = self
        if not callenv:
            callenv = env

        if obj.isA(Int) or obj.isA(Real) or obj.isA(String):
            return obj
        elif obj.isA(Quote) and obj.has('value'):
            return obj.get('value')
        elif obj.isA(Message):
            val = env.get(obj.value())
            if val.isCallable():
                args = Object.clone(*obj.slots)
                return val.call(env, args, callenv)
            else:
                if len(obj.slots):
                    raise FugaError, "can't pass arguments to non-method"
                return val
        elif obj.isA(List):
            if obj.get('empty?') is fgtrue:
                raise FugaError, "can't evaluate empty list"
            elif obj.get('single?') is fgtrue:
                return self.eval(obj.get('value'), env, callenv)
            else:
                env =  self.eval(obj.get('left'), env, callenv)
                return self.eval(obj.get('right'), env, callenv)
        else:
            newObj = fgobject(obj.proto)
            newEnv = fgobject(env)
            for (name, oldVal) in obj.slots:
                newVal = newEnv.eval(oldVal)
                newObj.set(name, newVal)
                newEnv.set(name, newVal)
            return newObj

    def isCallable(self):
        return self.isA(Method)

    def call(self, obj, args, argenv):
        if self.isA(Method):
            if self.value():
                return self.value()(obj, args, argenv)
            else:
                if len(self.slots) != 3: raise FugaError, "can only handle single methods at the moment."
                if len(self.get(1).slots) != len(args.slots):
                    raise FugaError, "method takes %s arguments, but %s given." % (len(self.get(1).slots), len(args.slots))

                scope = self.get('scope').clone(('self', obj))
                args  = argenv.eval(args)
                for i,(n,m) in enumerate(self.get(1).slots):
                    if not m.isA(Message):
                        raise FugaError, "can't handle non-msg parameters yet."
                    elif m.slots:
                        raise FugaError, "expected a msg with no args in formal parameter"
                    elif n:
                        raise FugaError, "can't handle named parameters yet."
                    else:
                        scope.set(m.value(), args.get(i))

                return scope.eval(self.get(2))
        else:
            raise FugaError, "expected a Method -- can't call a non-method"

    def __repr__(self):
        return self.eval(fgmsg('repr')).value()

    def __str__(self):
        return self.eval(fgmsg('str')).value()


Object = fgobject(None)

String = Object.clone()
def fgstr(value): return String.clone(value)

Bool    = Object.clone()
fgtrue  = Bool.clone()
fgfalse = Bool.clone()

Quote = Object.clone()
def fgquote(value):
    return Quote.clone(('value', value))

List = Object.clone(
    ('empty?', fgfalse),
    ('single?', fgfalse),
    ('conc?', fgfalse)
)
def fglist(*vals):
    if len(vals) == 0:
        return List.clone(
            ('len',     fgint(0)),
            ('empty?',  fgtrue),
        )
    elif len(vals) == 1:
        return List.clone(
            ('len',     fgint(1)),
            ('single?', fgtrue),
            ('value',   vals[0]),
        )
    else:
        mid = len(vals)/2
        return List.clone(
            ('len',     fgint(len(vals))),
            ('conc?',   fgtrue),
            ('left',    fglist(*vals[:mid])),
            ('right',   fglist(*vals[mid:]))
        )

Number = Object.clone()
Int = Number.clone()
Real = Number.clone()
def fgint(value): return Int.clone(value)
def fgreal(value): return Real.clone(value)


Message = Object.clone()
def fgmsg(value, *args): return Message.clone(value, *args)

Method = Object.clone()
def fgmethod(value): return Method.clone(value)



def Object_repr(self, args, env, depth=5):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self is Object: return fgstr('Object')
    if depth == 0: return fgstr("(...)")
    strs = []
    for (k,v) in self.slots:
        if v.get("repr").value() is Object_repr:
            vrepr = Object_repr(v, args, env, depth-1).value()
        else:
            vrepr = repr(v)
        if k is None:
            strs.append(vrepr)
        else:
            strs.append(k + "=" + vrepr)
    return fgstr("(%s)" % ', '.join(strs))
Object.set('str', fgmethod(lambda self,args,env:
    self.eval(fgmsg('repr'))))
Object.set('repr', fgmethod(Object_repr))

def String_repr(self, args, env):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self.value() is None: return fgstr('String')

    escape = {
        '\\': '\\\\',
        '\n': '\\n',
        '\r': '\\r',
        '\t': '\\t',
        '"': '\\"'
    }
    result = ''
    for c in self.value():
        if c in escape:
            result += escape[c]
        else:
            result += c
    return fgstr('"' + result + '"')
String.set('str',  fgmethod(lambda self,args,env:
    self if self.value() else fgstr('String')))
String.set('repr', fgmethod(String_repr))

Number.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value()) if self.value() is not None else 'Number')))
Int.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value()) if self.value() is not None else 'Int')))
Real.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value()) if self.value() is not None else 'Real')))
Bool.set('repr', fgstr('Bool'))
fgtrue.set('repr', fgstr('true'))
fgfalse.set('repr', fgstr('false'))


def Message_repr(self, args, env):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self.value() is None: return fgstr('Message')
    if self.value()[0] in ('abcdefghijklmnopqrstuvwxyz'
                        +'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                        +'0123456789'):
        name = self.value()
    else:
        name = '\\' + self.value()
    if len(self.slots):
        return fgstr(name + Object_repr(self, args, env).value())
    else:
        return fgstr(name)
Message.set('repr', fgmethod(Message_repr))

def List_repr(self, args, env):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self is List: return fgstr('List')
    if self.get('empty?') is fgtrue:
        return fgstr('')
    elif self.get('single?') is fgtrue:
        return fgstr(repr(self.get('value')))
    else:
        return fgstr(repr(self.get('left'))+' '+repr(self.get('right')))
List.set('repr', fgmethod(List_repr))

def Quote_repr(self, args, env):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self is Quote: return fgstr('Quote')
    return fgstr("'" + repr(self.get('value')))
Quote.set('repr', fgmethod(Quote_repr))

def Method_repr(self, args, env):
    if len(args.slots): raise FugaError, "repr expects 0 arguments"
    if self is Method: return fgstr('Method')
    return fgstr('method(...)')
Method.set('repr', fgmethod(Method_repr))
    

