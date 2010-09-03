
class ast(object):
    def __init__(self, *vargs, **kwargs):
        self.kwargs = kwargs
        self.cons(*vargs)

    def __repr__(self):
        return "ast(...)"

class block(ast):
    def cons(self, slots):
        self.slots = slots
    def __repr__(self): 
        reprslots = map(repr, self.slots)
        return "object(%s)" % ', '.join(reprslots) 


class slot(ast):
    def cons(self, left, slotop=None, right=None):
        if right is None:
            self.left   = None
            self.slotop = None
            self.right  = left
        else:
            self.left   = left
            self.slotop = slotop
            self.right  = right
        
    def __repr__(self):
        if self.slotop is None:
            return "slot(%r)" % self.right
        else:
            return "slot(%r, %r, %r)" % (self.left, self.slotop, self.right)

class exp(ast):
    def cons(self, root, msgs):
        self.root = root
        self.msgs = msgs

    def __repr__(self):
        a = [self.root] + self.msgs
        return "exp(%s)" % ', '.join(map(repr, a))

class msg(ast):
    def cons(self, name, args=None):
        self.name = name
        self.args = args

    def __repr__(self):
        if self.args is None:
            return "msg(%r)" % self.name
        else:    
            return "msg(%r, %r)" % (self.name, self.args)

class string(ast):
    def cons(self, value):
        self.value = value

    def __repr__(self):
        return "string(%r)" % self.value

class number(ast):
    def cons(self, value):
        self.value = value

    def __repr__(self):
        return "number(%r)" % self.value

