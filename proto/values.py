
class fgobject(object):
    def __init__(self, proto, *slots):
        if slots and not isinstance(slots[0], tuple):
            self.value = slots[0]
            slots = slots[1:]
        else:
            self.value = None
        self.proto = proto
        self.slots = list(slots)
        self.byName = {}
        for i,(k,v) in enumerate(slots):
            if k is not None:
                self.byName[k] = i

    def clone(self, *slots):
        return fgobject(self, *slots)

    def set(self, name, value):
        if isinstance(name, str) or name is None:
            if name in self.byName:
                raise KeyError, "Can't have two slots with name %s" % name
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
            raise KeyError, "No slot named %s in object." % name
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
            raise KeyError, "No slot named %s in object." % name

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
            val = env.get(obj.value)
            if val.isCallable():
                args = Object.clone(*obj.slots)
                return val.call(env, args, callenv)
            else:
                if len(obj.slots):
                    raise ValueError, "can't pass arguments to non-method"
                return val
        elif obj.isA(List):
            if obj.get('empty?') is fgtrue:
                raise ValueError, "can't evaluate empty list"
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
        if hasattr(self, 'value'):
            return self.value(obj, args, argenv)
        else:
            raise AttributeError, "expected a 'value' in Method"

    def __repr__(self):
        return self.eval(fgmsg('repr')).value


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



def Object_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is Object: return fgstr('Object')
    strs = []
    for (k,v) in self.slots:
        if k is None:
            strs.append(repr(v))
        else:
            strs.append(k + "=" + repr(v))
    return fgstr("(%s)" % ', '.join(strs))
Object.set('str', fgmethod(lambda self,args,env:
    self.eval(fgmsg('repr'))))
Object.set('repr', fgmethod(Object_repr))

def String_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is String: return fgstr('String')

    escape = {
        '\\': '\\\\',
        '\n': '\\n',
        '\r': '\\r',
        '\t': '\\t',
        '"': '\\"'
    }
    result = ''
    for c in self.value:
        if c in escape:
            result += escape[c]
        else:
            result += c
    return fgstr('"' + result + '"')
String.set('str',  fgmethod(lambda self,args,env:
    self if self.value else fgstr('String')))
String.set('repr', fgmethod(String_repr))

Number.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value) if self.value is not None else 'Number')))
Int.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value) if self.value is not None else 'Int')))
Real.set('repr', fgmethod(lambda self,args,env:
    fgstr(repr(self.value) if self.value is not None else 'Real')))
Bool.set('repr', fgstr('Bool'))
fgtrue.set('repr', fgstr('true'))
fgfalse.set('repr', fgstr('false'))


def Message_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is Message: return fgstr('Message')
    if self.value[0] in ('abcdefghijklmnopqrstuvwxyz'
                        +'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                        +'0123456789'):
        name = self.value
    else:
        name = '\\' + self.value
    if len(self.slots):
        return fgstr(name + Object_repr(self, args, env).value)
    else:
        return fgstr(name)
Message.set('repr', fgmethod(Message_repr))

def List_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is List: return fgstr('List')
    if self.get('empty?') is fgtrue:
        return fgstr('')
    elif self.get('single?') is fgtrue:
        return fgstr(repr(self.get('value')))
    else:
        return fgstr(repr(self.get('left'))+' '+repr(self.get('right')))
List.set('repr', fgmethod(List_repr))

def Quote_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is Quote: return fgstr('Quote')
    return fgstr("'" + repr(self.get('value')))
Quote.set('repr', fgmethod(Quote_repr))

def Method_repr(self, args, env):
    if len(args.slots): raise ValueError, "repr expects 0 arguments"
    if self is Method: return fgstr('Method')
    return fgstr('method(...)')
Method.set('repr', fgmethod(Method_repr))
    

# Some global functions...


def Object_get(self, args, env):
    if len(args.slots) != 1:
        raise ValueError, "get expects 1 argument"
    args = env.eval(args)
    name = args.slots[0][1]
    if name.isA(String): name = name.value
    elif name.isA(Message): name = name.value
    else: raise TypeError, "unexpected type for argument to get"
    return self.get(name)

def Object_set(self, args, env):
    if len(args.slots) != 1 and len(args.slots) != 2:
        raise ValueError, "set expects 1 or 2 arguments"
    args = env.eval(args)
    if len(args.slots) == 1:
        name = None
        value = args.slots[0][1]
    else:
        name = args.slots[0][1]
        value = args.slots[1][1]
        if name.isA(String): name = name.value
        elif name.isA(Message): name = name.value
        else: raise TypeError, "unexpected type for first argument to set" 
    self.set(name, value)
    return self

def Object_has(self, args, env):
    if len(args.slots) != 1:
        raise ValueError, "get expects 1 argument"
    args = env.eval(args)
    name = args.slots[0][1]
    if name.isA(String): name = name.value
    elif name.isA(Message): name = name.value
    else: raise TypeError, "unexpected type for argument to get"
    return fgtrue if self.has(name) else fgfalse

def Object_self(self, args, env):
    if len(args.slots) != 0:
        raise ValueError, "self expects no arguments"
    return self

def Object_proto(self, args, env):
    if len(args.slots) != 0:
        raise ValueError, "proto expects no arguments"
    return self.proto or Object

def Object_scope(self, args, env):
    if len(args.slots) != 0:
        raise ValueError, "scope expects no arguments"
    return env

# Set up some globals

Object.set('Object', Object)
Object.set('Number', Number)
Object.set('Int',    Int)
Object.set('Real',   Real)
Object.set('String', String)
Object.set('Bool',   Bool)
Object.set('true',   fgtrue)
Object.set('false',  fgfalse)
Object.set('get',    fgmethod(Object_get))
Object.set('set',    fgmethod(Object_set))
Object.set('has',    fgmethod(Object_has))
Object.set('self',   fgmethod(Object_self))
Object.set('proto',  fgmethod(Object_proto))
Object.set('scope',  fgmethod(Object_scope))

# Arithmetic would be nice

def Number_add(self, args, env):
    if len(args.slots) != 1:
        raise ValueError, "+ expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise TypeError, "Can not add non-numbers"
    if self.value is None: 
        raise ValueError, "Cannot add %r to a number." % self
    if args.get(0).value is None:
        raise ValueError, "Cannot add %r to a number." % args.get(0)
    if isinstance(self.value, int) and isinstance(args.get(0).value, int):
        return fgint(self.value + args.get(0).value)
    else:
        return fgreal(self.value + args.get(0).value)

def Number_sub(self, args, env):
    if len(args.slots) != 1:
        raise ValueError, "- expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise TypeError, "Can not subtract non-numbers"
    if self.value is None: 
        raise ValueError, "Cannot subtract a number from %r." % self
    if args.get(0).value is None:
        raise ValueError, "Cannot subtract %r from a number." % args.get(0)
    if isinstance(self.value, int) and isinstance(args.get(0).value, int):
        return fgint(self.value - args.get(0).value)
    else:
        return fgreal(self.value - args.get(0).value)

Number.set('+', fgmethod(Number_add))
Number.set('-', fgmethod(Number_sub))


if __name__ == '__main__':
    import doctest
    doctest.testmod()

