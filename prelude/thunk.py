
from .prelude import *

Prelude['Thunk'] = Thunk

@setmethod(Prelude, 'thunk')
def Prelude_thunk(self, args):
    args = args.splitThunk()
    if len(args) != 1:
        raise FugaError("thunk: expected 1 argument, got %d" % len(args))
    return fgfgthunk(args[0].code(), args[0].scope())

@setmethod(Thunk, 'value', 0)
def Thunk_eval(self):
    return self['code'].eval(self['scope'])

@setmethod(Thunk, 'slots', 0)
def Thunk_slots(self):
    result = Object.clone()
    for k in self['code']._slots:
        result[k] = fgfgthunk(self['code'][k], self['scope'])
    return result

