from .core import *

Prelude = Object.clone()
Prelude['Prelude'] = Prelude
Prelude['name'] = fgstr('Prelude')

Prelude['Object'] = Object
Prelude['Int']    = Int
Prelude['String'] = String
Prelude['Symbol'] = Symbol
Prelude['Msg']    = Msg
Prelude['Method'] = Method
Prelude['Expr']   = Expr
Prelude['Bool']   = Bool

Prelude['true']   = fgtrue
Prelude['false']  = fgfalse

@setmethod(Prelude, 'scope')
def Prelude_scope(self, args):
    scope = args.scope()
    if len(args):
        raise FugaError("Prelude scope: expected 0 arguments, got %d" %
            len(args))
    return scope

@setmethod(Prelude, 'do')
def Prelude_do(self, args):
    args.thunkSlots(True)
    args.needSlots()
    return args[len(args)-1]

@setmethod(Prelude, 'method')
def Prelude_method(self, args):
    pass

