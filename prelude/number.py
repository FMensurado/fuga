from .prelude import *

Prelude["Number"] = Number
Prelude["Int"] = Int

# integer arithmetic

@setmethod(Int, '+', 1)
def Int_plus(self, other):
    if not other.isa(Int):
        raise FugaError("Int +: right-hand argument must be an Int")
    return fgint(self.value() + other.value())

@setmethod(Int, '-', 1)
def Int_minus(self, other):
    if not other.isa(Int):
        raise FugaError("Int -: right-hand argument must be an Int")
    return fgint(self.value() - other.value())

@setmethod(Int, '*', 1)
def Int_times(self, other):
    if not other.isa(Int):
        raise FugaError("Int *: right-hand argument must be an Int")
    return fgint(self.value() * other.value())

@setmethod(Int, '//', 1)
def Int_floordiv(self, other):
    if not other.isa(Int):
        raise FugaError("Int //: right-hand argument must be an Int")
    return fgint(self.value() // other.value())

@setmethod(Int, '%', 1)
def Int_modulo(self, other):
    if not other.isa(Int):
        raise FugaError("Int //: right-hand argument must be an Int")
    return fgint(self.value() % other.value())

# comparisons

@setmethod(Int, '==', 1)
def Int_eq(self, other):
    if not other.isa(Int):
        raise FugaError("Int ==: right-hand argument must be an Int")
    return fgbool(self.value() == other.value())

@setmethod(Int, '<', 1)
def Int_lt(self, other):
    if not other.isa(Int):
        raise FugaError("Int <: right-hand argument must be an Int")
    return fgbool(self.value() < other.value())

@setmethod(Int, '>', 1)
def Int_gt(self, other):
    if not other.isa(Int):
        raise FugaError("Int >: right-hand argument must be an Int")
    return fgbool(self.value() > other.value())

@setmethod(Int, '<=', 1)
def Int_le(self, other):
    if not other.isa(Int):
        raise FugaError("Int <=: right-hand argument must be an Int")
    return fgbool(self.value() <= other.value())

@setmethod(Int, '>=', 1)
def Int_ge(self, other):
    if not other.isa(Int):
        raise FugaError("Int >=: right-hand argument must be an Int")
    return fgbool(self.value() >= other.value())
