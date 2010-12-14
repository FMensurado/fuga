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
