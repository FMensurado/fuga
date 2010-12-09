
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
        self.slots = {}
        self.length = 0
        self.thunk = None
        for (k,v) in slots:
            self.set(k,v)
            if k:
                self.slots[k] = v
            else:
                self.slots[self.length] = v
                self.length += 1
 
    def mkStrict(self):
        while self.thunk:
            result = self.thunk[0].eval(self.thunk[1])
            self. slots  = result. slots
            self. length = result. length
            self. proto  = result. proto
            self._value  = result._value
            self. thunk  = result. thunk

    def value(self):
        self.mkStrict()
        if self._value is not None:
            return self._value
        elif self.proto:
            return self.proto.value()
        else:
            return None

    def clone(self, *slots):
        return fgobject(self, *slots)

    def set(self, name, value):
        self.mkStrict()
        if name is None:
            self.slots[self.length] = value
            while self.length in self.slots:
                self.length += 1
        elif isinstance(name, int) or isinstance(name, str):
            self.slots[name] = value
            while self.length in self.slots:
                self.length += 1
        else:
            raise TypeError, "Can't set slot by type %s." % type(name)

    def rawHas(self, name):
        self.mkStrict()
        if isinstance(name, str) or isinstance(name, int):
            return name in self.slots
        else:
            raise TypeError, "Can't get slot by type %s." % type(name)

    def has(self, name):
        return self.rawHas(name) or (self.proto and self.proto.has(name))

    def rawGet(self, name):
        if not self.rawHas(name):
            raise FugaError, "No slot named %s" % name
        return self.slots[name]

    def get(self, name):
        if self.rawHas(name):
            return self.rawGet(name)
        elif self.proto and self.proto.has(name):
            return self.proto.get(name)
        else:
            raise FugaError, "No slot named %s" % name

    def isA(self, proto):
        self.mkStrict()
        return self.proto and (self.proto is proto
                           or  self.proto.isA(proto))

    def eval(self, obj, scope=None):
        if not scope:
            scope = self
        if obj.isA(Int) or obj.isA(String):
            return obj
        elif obj.isA(Quote):
            return obj.get('value')
        elif obj.isA(List):
            pass ## TODO
            raise FugaError, "not yet implemented: Expr"
        elif obj.isA(Message):
            fn   = self.get(obj.value())
            args = Object.clone()
            for i in obj.slots:
                args.set(i, obj.get(i))
            fn.call(self, args, scope)
        else:
            return self.evalParts(obj)

    def evalParts(self, obj):
        obj.mkStrict()
        env = self.clone()
        result = self.clone()
        for k in obj.slots:
            if isinstance(k, str) or k >= obj.length:
                val = env.mkThunk(obj.get(k))
                env.set(k, val)
                result.set(k, val)
        for i in range(obj.length):
            result.set(i, env.eval(obj.get(i), env))

    def mkThunk(self, obj):
        result = fgobject(obj.proto)
        result.slot[0][1]

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

def Object_str(self, args, env, depth=5):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
    if self is Object: return fgstr('Object')
    if depth == 0: return fgstr("(...)")
    strs = []
    for (k,v) in self.slots:
        if v.get("str").value() is Object_str:
            vstr = Object_str(v, args, env, depth-1).value()
        else:
            vstr = str(v)
        if k is None:
            strs.append(vstr)
        else:
            strs.append(k + "=" + vstr)
    return fgstr("(%s)" % ', '.join(strs))
Object.set('str', fgmethod(Object_str))

def String_str(self, args, env):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
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
String.set('str', fgmethod(String_str))

Number.set('str', fgmethod(lambda self,args,env:
    fgstr(str(self.value()) if self.value() is not None else 'Number')))
Int.set('str', fgmethod(lambda self,args,env:
    fgstr(str(self.value()) if self.value() is not None else 'Int')))
Real.set('str', fgmethod(lambda self,args,env:
    fgstr(str(self.value()) if self.value() is not None else 'Real')))
Bool.set('str', fgstr('Bool'))
fgtrue.set('str', fgstr('true'))
fgfalse.set('str', fgstr('false'))

def Message_str(self, args, env):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
    if self.value() is None: return fgstr('Message')
    if self.value()[0] in ('abcdefghijklmnopqrstuvwxyz'
                          +'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                          +'0123456789'):
        name = self.value()
    else:
        name = '\\' + self.value()
    if len(self.slots):
        return fgstr(name + Object_str(self, args, env).value())
    else:
        return fgstr(name)
Message.set('str', fgmethod(Message_str))

def List_str_aux(self):
    if not self.isA(List):
        raise FugaError, "list instance not composed of list instances!"
    if self.get('empty?') is fgtrue:
        return ''
    elif self.get('single?') is fgtrue:
        return str(self.get('value'))
    else:
        left  = List_str_aux(self.get('left'))
        right = List_str_aux(self.get('right'))
        if left and right:
            return left + ", " + right
        else:
            return left + right
def List_str(self, args, env):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
    if self is List: return fgstr('List')
    return fgstr("list(" + List_str_aux(self) + ")")
List.set('str', fgmethod(List_str))

def Quote_str(self, args, env):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
    if self is Quote: return fgstr('Quote')
    return fgstr("'" + str(self.get('value')))
Quote.set('str', fgmethod(Quote_str))

def Method_str(self, args, env):
    if len(args.slots): raise FugaError, "str expects 0 arguments"
    if self is Method: return fgstr('Method')
    scopeless = Object.clone(*self.slots[1:])
    return fgstr('method(...)')
Method.set('str', fgmethod(Method_str))
Object.set('Method', Method)

# Join -- for (simulated) multiple inheritance.

class fgjoin(fgobject):
    def __init__(self, left, right):
        self.left = left
        self.right = right
        super(fgjoin, self).__init__(self, left)

    def doneWithFeeler(self, feelers):
        for feeler in feelers[1:]:
            if feeler is feelers[0] or feeler.isA(feelers[0]):
                return True
        return False

    def get(self, name):
        if self.rawHas(name):
            return self.rawGet(name)
        if not self.has(name):
            return self.left.get(name) # raises error
        feelers = [self.left, self.right]
        while feelers:
            if isinstance(feelers[0], fgjoin):
                feelers = [feelers[0].left, feelers[0].right] + feelers[1:]
            elif self.doneWithFeeler(feelers):
                del feelers[0]
            elif feelers[0].rawHas(name):
                return feelers[0].rawGet(name)
            elif feelers[0] is Object:
                break
            else:
                feelers[0] = feelers[0].proto
        raise ValueError, "logic error: expected slot by now!"

    def has(self, name):
        return (self.rawHas(name) or self.left.has(name)
                                  or self.right.has(name))

    def isA(self, obj):
        if obj is self.left or obj is self.right:
            return True
        return obj.isA(self.left) or obj.isA(self.right)

def Object_join(self, args, env):
    if args.length != 2:
        raise FugaError, "join expects exactly 2 arguments (for now)"
    args = env.eval(args)
    return fgjoin(args.get(0), args.get(1))

Object.set("join", fgmethod(Object_join))

# void and null

fgvoid = Object.clone()
Object.set("void",  fgvoid)
Object.set("void?", fgfalse)
fgvoid.set("void?", fgtrue)
fgvoid.set("str", fgstr("void"))
 
fgnull = Object.clone()
Object.set("null",  fgnull)
Object.set("null?", fgfalse)
fgnull.set("null?", fgtrue)
fgnull.set("str", fgstr("null"))
fgnull.set("proto", fgnull)

# Variables

Var = Object.clone()
def fgvar(value):
    return Var.clone(('value', value))

def Object_var(self, args, env):
    if args.length > 1:
        raise FugaError, "var expects 0 or 1 arguments"
    if args.slots:
        args = env.eval(args)
        return fgvar(args.get(0))
    else:
        return fgvar(fgnull)

def Var_set(self, args, env):
    if args.length != 1:
        raise FugaError, "Var set! expects 1 argument."
    if not self.isA(Var):
        raise FugaError, "can't use Var set! on Var, only on instances"
    args = env.eval(args)

    self.slots[0] = ('value', args.get(0))
    return fgvoid

def Object_varset(self, args, env):
    if args.length != 2:
        raise FugaError, ":= expects 2 arguments."
    left = args.get(0)
    if left.isA(Message):
        name = left.value()
        left = env.eval(fgmsg("get", (None, fgstr(name))))
    else:
        raise FugaError, ":= doesn't support non-msg left argument yet"
    return Var_set(left, Object.clone((None, args.get(1))), env)

Var.set("set!", fgmethod(Var_set))

Object.set("Var", Var)
Object.set("var", fgmethod(Object_var))
Object.set(":=", fgmethod(Object_varset))


