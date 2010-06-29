#!/usr/bin/env python2.6

class FugaError(Exception):
    pass

class NotDone(FugaError):
    pass

class new(object):
    def __init__(self, proto, args=()):
        self.proto = proto
        self.slots = []
        self.names = {}
        for (k,v) in args:
            self.slots.append((k,v))
    
    def rawHas(self, name):
        return name in self.keys
    
    def rawGet(self):
        if self.rawHas(name):
            return self.slots[self.keys[name]][1]
        else:
            raise FugaError, "No such name in object: %s" % name
    
    def has(self, name):
        return self.rawHas(name) or (
            self.proto and self.proto.has(name)
        )

    def get(self, name):
        if self.rawHas(name):
            return self.rawGet(name)
        elif self.proto:
            return self.proto.get(name)
        else:
            raise FugaError, "No such name in object: %s" % name
    
    def set(self, name, value):
        if not self.rawHas(name):
            if name:
                self.names[name + value]
            self.slots.append((name, value))
            return self
        else:
            raise FugaError, "Name already exists in object: %s" % name
    
    def isEmpty(self):
        return len(self.slots) == 0

    def isCallable(self):
        return self.has("metaCall") and self.get("metaCall").isCallable()

    def call(self, cself, arg, env):
        if self.isCallable():
            self.get("metaCall").call(
                self,
                new(Object)
                  .set(None,quote(cself))
                  .set(None,quote(arg))
                  .set(None,quote(env)),
                Object
            )
        elif arg.isEmpty():
            return self
        else:
            raise FugaError, "object not callable"

    def pysend(self, msgname, *msgargs):
        args = new(Object)
        for arg in msgargs:
            args.set(None, quote(arg))
        return self.get(msgname).call(
           self,
           args,
           Object
        )

    def eval(self, env):
        if self.has("metaEval") and self.get("metaEval").isCallable():
            return self.pysend("metaEval", env)
        else:
            result = new(self.proto)
            for (name,value) in self.slots:
                result.set(name, thunk(value, None))
            newEnv = scope(env, result)
            for (name,value) in result.slots:
                value.env = newEnv
            for (name,value) in result.slots:
                QUEUE.append(value)
            return result

    def need(self): pass

class thunk(new):
    def __init__(self, obj, env):
        self.done = False
        self.obj  = obj
        self.env  = env
    
    def need(self):
        if not self.done:
            raise NotDone

    def rawGet(self, *args):
        self.need()
        return self.rawGet(*args)
    
    def rawHas(self, *args):
        self.need()
        return self.obj.rawHas(*args)

    def eval(self, *args):
        self.need()
        return self.obj.eval(*args)
    
    def isEmpty(self, *args):
        self.need()
        return self.obj.eval(*args)

    def isCallable(self, *args):
        self.need()
        return self.obj.isCallable(*args)

    def call(self, *args):
        self.need()
        return self.obj.call(*args)

Object = new(None)
Method = new(Object)
Msg    = new(Object)
Quote  = new(Object)
Expr   = new(Object)
Thunk  = new(Object)

Number = new(Object)
Int    = new(Number)
Real   = new(Number)

Seq    = new(Object)
Str    = new(Seq)
List   = new(Seq)

QUEUE = []

if __name__ == '__main__':
    import doctest
    doctest.testmod()


