from .prelude import *
import sys

File = Prelude['File'] = Object.clone()
File['stdout'] = File.clone(sys.stdout)
File['stdin']  = File.clone(sys.stdin)
File['stderr'] = File.clone(sys.stderr)

@setmethod(File, 'print')
def File_print(self, args):
    if not self.isPrimitive():
        raise FugaError("File print: self must be a primitive file")
    vals = []
    for slot in args:
        if slot.isa(String) and slot.isPrimitive(str):
            vals.append(slot.value())
        else:
            vals.append(repr(slot))
    print(*vals, file=self.value())
    return Object

@setmethod(Prelude, 'print')
def Prelude_print(self, args):
    return File_print(File['stdout'], args)

