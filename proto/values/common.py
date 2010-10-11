
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
            raise FugaError, "No slot named %s" % name
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
            raise FugaError, "No slot named %s" % name

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

    def call(method, self, actuals, env, guard=False):
        if method.isA(Method):
            if method.value():
                return method.value()(self, actuals, env)
           
            index = 1
            scope   = method.get('scope')
            while index < len(method.slots):
                if method.slots[index][0] is not None:
                    continue
                try:
                    if method.get(index).isA(Method):
                        m = method.get(index)
                        index += 1
                        return m.call(self, actuals, env, True)
                    else:
                        formals = method.get(index)
                        body    = method.get(index+1)
                        index += 2
                        if len(formals.slots) == len(actuals.slots):
                            actuals = lazy_eval_actuals(actuals, env)
                            return handle_call(
                                formals,
                                actuals,
                                body,
                                scope,
                                self
                            )
                except FugaError, e:
                    if 'GUARD' in str(e):
                        pass
                    else:
                        raise e
            
            if guard:
                raise FugaError, "GUARD"
            else:
                raise FugaError, "No pattern matches arguments to method."
        else:
            raise FugaError, "expected a Method -- can't call a non-method"

    def __repr__(self):
        return self.eval(fgmsg('str')).value()

class lazy_eval_actuals(object):
    """Ensure that actuals are evaluated only if value needs to bound,
    while also ensuring that actuals are only evaluated once."""
    def __init__(self, actuals, env):
        self.values  = {}
        self.actuals = actuals
        self.env     = env

    def get_value(self, name):
        if name not in self.values:
            value = self.env.eval(self.actuals.get(name))
            self.values[name] = value
            if isinstance(name, int):
                if self.actuals.slots[name][0]:
                    self.values[self.actuals.slots[name][0]] = value
            else:
                self.value[self.actuals.byName[name]] = value
        return self.values[name]

    def get_code(self, name):
        return self.actuals.get(name)

def handle_call(formals, actuals, body, scope, self):
    scope = scope.clone(('self', self))
    for i,(n,m) in enumerate(formals.slots):
        if m.isA(Quote):
            scope.set('caller', actuals.env)
            scope.set(m.get('value').value(), actuals.get_code(i))
        elif not m.isA(Message):
            if m.eval(fgmsg(
                '==',
                (None, fgquote(actuals.get_value(i)))
            )) is not fgtrue:
                raise FugaError, "GUARD"
        elif m.slots:
            raise FugaError, "in formal parameter, expected just a name"
        elif n:
            raise FugaError, "can't handle named parameters yet."
        else:
            scope.set(m.value(), actuals.get_value(i))
    return scope.eval(body)

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
    return fgstr('method(...)')
Method.set('str', fgmethod(Method_str))
    

