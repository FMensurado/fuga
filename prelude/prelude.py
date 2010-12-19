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
    args.thunkSlots(True)
    args.needSlots()
    return args[len(args)-1]

@setmethod(Prelude, 'if')
def Prelude_if(self, args):
    args.thunkSlots()
    if len(args) != 3:
        raise FugaError("if: expected 3 arguments, got $d" % len(args))
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

