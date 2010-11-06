
from common import *

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

