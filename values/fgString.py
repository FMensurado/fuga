
from common import *

# String methods

def String_len(self, args, env):
    if len(args.slots) != 0:
        raise FugaError, "`String len` requires exactly 0 requirements."
    if self.value() is None:
        raise FugaError, "`String len` can only be sent to instances of String -- not to String."
    return fgint(len(self.value()))

def String_find(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "`String find` requires exactly 1 requirement."
    if self.value() is None:
        raise FugaError, "`String find` can only be sent to instances of String -- not to String."
    args = env.eval(args)
    if not args.get(0).isA(String) or args.get(0).value() is None:
        raise FugaError, "Argument to `String find` must be a string."
    return fgint(self.value().find(args.get(0).value()))

def String_slice(self, args, env):
    if len(args.slots) < 1 or len(args.slots) > 2:
        raise FugaError, "`String slice` requires 1 or 2 arguments."
    if self.value() is None:
        raise FugaError, "`String slice` can only be sent to instances of String -- not to String."
    args = env.eval(args)
    if not args.get(0).isA(Int) or args.get(0).value() is None:
        raise FugaError, "1st argument to `String slice` must be an Int."
    if len(args.slots) == 1:
        return fgstr(self.value()[args.get(0).value():])
    if not args.get(1).isA(Int) or args.get(1).value() is None:
        raise FugaError, "2nd argument to `String slice` must be an Int."
    return fgstr(self.value()[args.get(0).value():args.get(1).value()])

String.set('len',   fgmethod(String_len))
String.set('find',  fgmethod(String_find))
String.set('slice', fgmethod(String_slice))


