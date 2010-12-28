from .core import *

Prelude = Object.clone()
Prelude['Prelude'] = Prelude
Prelude['name'] = fgstr('Prelude')

Prelude['Object'] = Object
Prelude['String'] = String
Prelude['Symbol'] = Symbol
Prelude['Msg']    = Msg
Prelude['Method'] = Method
Prelude['Expr']   = Expr
Prelude['Bool']   = Bool
Prelude['void']   = void

Prelude['true']   = fgtrue
Prelude['false']  = fgfalse

@setmethod(Prelude, 'scope')
def Prelude_scope(self, args):
    scope = args.scope()
    if len(args):
        raise FugaError("scope: expected 0 arguments, got %d" %
            len(args))
    return scope

@setmethod(Prelude, 'do')
def Prelude_do(self, args):
    args = args.splitThunk(True)
    args.needSlots()
    return args[len(args)-1]

@setmethod(Prelude, 'if')
def Prelude_if(self, args):
    args = args.splitThunk(True)
    if len(args) != 3:
        raise FugaError("if: expected 3 arguments, got %d" % len(args))
    args[0].need()
    if args[0].is_(fgtrue):
        args[1].need()
        return args[1]
    elif args[0].is_(fgfalse):
        args[2].need()
        return args[2]
    raise FugaError("if: first argument must be a true or false.")

@setmethod(Prelude, 'method')
def Prelude_method(self, args):
    method = Method.clone(scope=args.scope())
    method.extend(args.code())
    return method

@setmethod(Prelude, '.')
def Prelude_dot(self, args):
    args = args.splitThunk()
    if len(args) != 2:
        raise FugaError(". expected 2 arguments, got %d" % len(args))
    recv = args[0]
    code = args[1].code()
    scope = args[1].scope()
    if code.isPrimitive(str) or code.isPrimitive(int):
        return recv.get(code.value())
    elif code.isa(Expr):
        head = code[0]
        rest = Expr.clone()
        for i,v in enumerate(code):
            if i != 0:
                rest.append(v)
        recv = Prelude_dot(self, Object.clone(recv, fgthunk(head, scope)))
        return rest.eval(recv, scope)
    else:
        raise FugaError(". unknown/unacceptable type for second arg")

