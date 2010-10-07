from common import *

# Arithmetic Operators

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

