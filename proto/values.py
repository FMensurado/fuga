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
            raise FugaError, "No slot named %s in object." % name
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
            raise FugaError, "No slot named %s in object." % name

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
    

# Some global functions...


def Object_get(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "get expects 1 argument"
    args = env.eval(args)
    name = args.slots[0][1]
    if name.isA(String): name = name.value()
    elif name.isA(Message): name = name.value()
    elif name.isA(Int): name = name.value()
    else: raise FugaError, "unexpected type for argument to get"
    return self.get(name)

def Object_set(self, args, env):
    if len(args.slots) != 1 and len(args.slots) != 2:
        raise FugaError, "set expects 1 or 2 arguments"
    args = env.eval(args)
    if len(args.slots) == 1:
        name = None
        value = args.slots[0][1]
    else:
        name = args.slots[0][1]
        value = args.slots[1][1]
        if name.isA(String): name = name.value()
        elif name.isA(Message): name = name.value()
        else: raise FugaError, "unexpected type for first argument to set" 
    self.set(name, value)
    return self

def Object_has(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "get expects 1 argument"
    args = env.eval(args)
    name = args.slots[0][1]
    if name.isA(String): name = name.value()
    elif name.isA(Message): name = name.value()
    else: raise FugaError, "unexpected type for argument to get"
    return fgtrue if self.has(name) else fgfalse

def Object_proto(self, args, env):
    if len(args.slots) != 0:
        raise FugaError, "proto expects no arguments"
    return self.proto or Object

def Object_scope(self, args, env):
    if len(args.slots) != 0:
        raise FugaError, "scope expects no arguments"
    return env

# Set up some globals

Object.set('Object', Object)
Object.set('Number', Number)
Object.set('Int',    Int)
Object.set('Real',   Real)
Object.set('String', String)
Object.set('List',   List)
Object.set('Bool',   Bool)
Object.set('true',   fgtrue)
Object.set('false',  fgfalse)
Object.set('get',    fgmethod(Object_get))
Object.set('set',    fgmethod(Object_set))
Object.set('has',    fgmethod(Object_has))
Object.set('proto',  fgmethod(Object_proto))
Object.set('scope',  fgmethod(Object_scope))

# Arithmetic would be nice

def Number_add(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "+ expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise FugaError, "Can not add non-numbers"
    if self.value() is None: 
        raise FugaError, "Cannot add %r to a number." % self
    if args.get(0).value() is None:
        raise FugaError, "Cannot add %r to a number." % args.get(0)
    if isinstance(self.value(), int) and isinstance(args.get(0).value(), int):
        return fgint(self.value() + args.get(0).value())
    else:
        return fgreal(self.value() + args.get(0).value())

def Number_sub(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "- expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise FugaError, "Can not subtract non-numbers"
    if self.value() is None: 
        raise FugaError, "Cannot subtract a number from %r." % self
    if args.get(0).value() is None:
        raise FugaError, "Cannot subtract %r from a number." % args.get(0)
    if isinstance(self.value(), int) and isinstance(args.get(0).value(), int):
        return fgint(self.value() - args.get(0).value())
    else:
        return fgreal(self.value() - args.get(0).value())

def Number_mul(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "* expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise FugaError, "Can not multiply non-numbers"
    if self.value() is None: 
        raise FugaError, "Cannot multiply %r." % self
    if args.get(0).value() is None:
        raise FugaError, "Cannot multiply %r." % args.get(0)
    if isinstance(self.value(), int) and isinstance(args.get(0).value(), int):
        return fgint(self.value() * args.get(0).value())
    else:
        return fgreal(self.value() * args.get(0).value())

def Number_div(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "/ expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise FugaError, "Can not divide by a non-number."
    if self.value() is None: 
        raise FugaError, "Cannot divide %r." % self
    if args.get(0).value() is None:
        raise FugaError, "Cannot divide by %r." % args.get(0)
    return fgreal(float(self.value()) / args.get(0).value())

def Number_fdiv(self, args, env):
    import math
    if len(args.slots) != 1:
        raise FugaError, "// expects an argument"
    value = Number_div(self, args, env).value()
    return fgint(int(math.floor(value)))

def Number_mod(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "% expects an argument"
    args = env.eval(args)
    if not args.get(0).isA(Number):
        raise FugaError, "Can not modulo by a non-number."
    if self.value() is None: 
        raise FugaError, "Cannot modulo by %r." % self
    if args.get(0).value() is None:
        raise FugaError, "Cannot module %r." % args.get(0)
    if isinstance(self.value(), int) and isinstance(args.get(0).value(), int):
        return fgint(self.value() % args.get(0).value())
    else:
        return fgreal(self.value() % args.get(0).value())

Number.set('+', fgmethod(Number_add))
Number.set('-', fgmethod(Number_sub))
Number.set('*', fgmethod(Number_mul))
Number.set('/', fgmethod(Number_div))
Number.set('//', fgmethod(Number_fdiv))
Number.set('%', fgmethod(Number_mod))

# Comparison Operators

def mkcomparer(name, fn):
    def comparer(self, args, env):
        if len(args.slots) != 1:
            raise FugaError, "%s expects 1 argument (%d given)" % (
                name,
                len(args.slots)
            )
        args = env.eval(args)
        if not args.get(0).isA(Number):
            raise FugaError, "can't compare number with non-number."
        if self.value() is None:
            raise FugaError, "can't compare %r with a number" % self
        if args.get(0).value() is None:
            raise FugaError, "can't compare %r with a number" %args.get(0)
        if fn(self.value(), args.get(0).value()):
            return fgtrue
        else:
            return fgfalse
    Number.set(name, fgmethod(comparer))


mkcomparer('<',  lambda x,y: x <  y)
mkcomparer('<=', lambda x,y: x <= y)
mkcomparer('>',  lambda x,y: x >  y)
mkcomparer('>=', lambda x,y: x >= y)
mkcomparer('==', lambda x,y: x == y)
mkcomparer('!=', lambda x,y: x != y)

# Boolean operators

def fgtrue_and(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "'and' can only support exactly 1 argument"
    result = env.eval(args.get(0))
    if result.isA(Bool):
        return result
    else:
        raise FugaError, "argument to 'and' must evaluate to a Bool"

def fgfalse_and(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "'and' can only support exactly 1 argument"
    return fgfalse

def fgtrue_or(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "'or' can only support exactly 1 argument"
    return fgtrue
        
def fgfalse_or(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "'or' can only support exactly 1 argument"
    result = env.eval(args.get(0))
    if result.isA(Bool):
        return result
    else:
        raise FugaError, "argument to 'or' must evaluate to a Bool"

fgtrue .set('and', fgmethod(fgtrue_and))
fgtrue .set('or',  fgmethod(fgtrue_or))
fgfalse.set('and', fgmethod(fgfalse_and))
fgfalse.set('or',  fgmethod(fgfalse_or))

# Some Important Methods!

def Object_method(self, args, env):
    if len(args.slots) != 2:
        raise FugaError, "method can only support exactly 2 arguments at the moment"
    return Method.clone(
        ('scope', self),
        (None, args.get(0)),
        (None, args.get(1))
    )
Object.set("method", fgmethod(Object_method))

def Object_clone(self, args, env):
    return self.clone(*env.eval(args).slots)
Object.set("clone", fgmethod(Object_clone))

# Some Control Flow

def Object_if(self, args, env):
    if len(args.slots) !=  3:
        raise FugaError, "if expects 3 arguments exactly"
    value = env.eval(args.get(0))
    if value is fgtrue:
        return env.eval(args.get(1))
    elif value is fgfalse:
        return env.eval(args.get(2))
    else:
        raise FugaError, "if expects a Bool, not %s" % value
Object.set("if", fgmethod(Object_if))

# List functions

def Object_list(self, args, env):
    args = env.eval(args)
    args = [args.get(i) for i in range(len(args.slots))]
    return fglist(*args)
Object.set("list", fgmethod(Object_list))

def List_map(self, args, env):
    if len(args.slots) > 2 or len(args.slots) == 0:
        raise FugaError, "`List map` expects 1 or 2 arguments."
    if len(args.slots) == 2:
        args = Object.clone((None,
            Method.clone(
                ('scope', env),
                (None, Object.clone((None, args.get(0)))),
                (None, args.get(1))
            )
        ))
    else:
        args = env.eval(args)

    if self.get('empty?') is fgtrue:
        return fglist()
    elif self.get('single?') is fgtrue:
        return fglist(
            args.get(0).call(self, 
                Object.clone((None, fgquote(self.get('value'))))
            , env)
        )
    elif self.get('conc?') is fgtrue:
        new_args = Object.clone((None, fgquote(args.get(0))))
        return List.clone(
            ('conc?', fgtrue),
            ('left',  List_map(self.get('left'),  new_args, env)),
            ('right', List_map(self.get('right'), new_args, env))
        )
    else:
        raise FugaError, "List map needs list instance."

List.set("map", fgmethod(List_map))

def List_conc(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "`List \++` requires 1 argument."
    args = env.eval(args)
    return List.clone(
        ('conc?', fgtrue),
        ('left',  self),
        ('right', args.get(0))
    )

List.set('++', fgmethod(List_conc))

if __name__ == '__main__':
    import doctest
    doctest.testmod()

