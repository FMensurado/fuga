"""
>>> import parser
>>> parser.parse('10').convert()
10
>>> parser.parse('"Hello, world!"').convert()
"Hello, world!"
>>> parser.parse('()').convert().slots
[]
>>> parser.parse('(1, 2, 3)').convert()
(1, 2, 3)
>>> parser.parse('foo = 845').convert()
(foo=845)
>>> parser.parse('foo(x) => 845').convert()
(foo=method((x), 845))
>>> parser.parse('x y = z').convert()
x set('y, z)
>>> parser.parse("foo bar").convert()
foo bar
>>> parser.parse("`bar baz('a b c)").convert()
get('bar) baz('a b c)
>>> parser.parse("a + b").convert()
a \+(b)
>>> parser.parse("a + b - c").convert()
a \+(b) \-(c)
>>> parser.parse("a + b * c").convert()
a \+(b \*(c))
"""
import values
import operators

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

    def convert(self):
        result = values.Object.clone()
        for cur_slot in self.slots:
            if isinstance(cur_slot, slot):
                result.set(cur_slot.left.name, cur_slot.right.convert())
            else:
                result.set(None, cur_slot.convert())
        return result

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
    
    def normalized(self):
        if self.slotop == '=>':
            self.slotop = '='
            if isinstance(self.left, exp):
                last_args = self.left.msgs[-1].args
                self.left.msgs[-1].args = None
            else:
                last_args = self.left.args
                self.left.args = None
            if not last_args:
                last_args = block([])
            self.right = msg('method',
                block([last_args, self.right])
            )

        if isinstance(self.left, exp):
            name = self.left.msgs[-1]
            return exp(self.left.root, self.left.msgs[:-1] +
                [msg('set', block([quote(name), self.right]))])
        else:
            return self

    def __repr__(self):
        if self.slotop is None:
            return "slot(%r)" % self.right
        else:
            return "slot(%r, %r, %r)" % (self.left, self.slotop, self.right)

    def convert(self):
        # This will only happen at a top-level.Usually in the interpereter.
        result = self.normalized()
        if not isinstance(result, slot): return result
        return msg('set', block([quote(self.left), self.right])).convert()

class opexp(ast):
    def cons(self, parts, ops):
        self.parts = parts
        self.ops = ops
    
    def __repr__(self):
        return "opexp(%r, %r)" % (self.parts, self.ops)

    def flatten(self, groups):
        def combine(left, op, right):
            newmsg = msg(op, block([right]))
            if isinstance(left, exp):
                left.msgs.append(newmsg)
                return left
            else:
                return exp(left, [newmsg])
        return operators.solve(groups, self.parts, self.ops, combine)

    def convert(self, groups=None):
        if not groups: groups = operators.std_groups
        return self.flatten(groups).convert()

class exp(ast):
    def cons(self, root, msgs):
        self.root = root
        self.msgs = msgs

    def __repr__(self):
        a = [self.root] + self.msgs
        return "exp(%s)" % ', '.join(map(repr, a))

    def convert(self):
        p = [self.root.convert()] + map(lambda m: m.convert(), self.msgs)
        return values.fglist(*p)

class msg(ast):
    def cons(self, name, args=None):
        self.name = name
        self.args = args

    def __repr__(self):
        if self.args is None:
            return "msg(%r)" % self.name
        else:
            return "msg(%r, %r)" % (self.name, self.args)

    def convert(self):
        if self.args is None:
            return values.fgmsg(self.name)
        else:
            return values.fgmsg(self.name, *self.args.convert().slots)

class string(ast):
    def cons(self, value):
        self.value = value

    def __repr__(self):
        return "string(%r)" % self.value

    def convert(self):
        return values.fgstr(self.value)

class number(ast):
    def cons(self, value):
        self.value = value

    def __repr__(self):
        return "number(%r)" % self.value

    def convert(self):
        if isinstance(self.value, int):
            return values.fgint(self.value)
        else:
            return values.fgreal(self.value)

class quote(ast):
    def cons(self, value):
        self.value = value

    def __repr__(self):
        return "quote(%r)" % self.value

    def convert(self):
        return values.fgquote(self.value.convert())

if __name__ == '__main__':
    import doctest
    doctest.testmod()

