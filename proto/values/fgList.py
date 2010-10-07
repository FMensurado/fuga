
from common import *

# List functions

def Object_list(self, args, env):
    args = env.eval(args)
    args = [args.get(i) for i in range(len(args.slots))]
    return fglist(*args)
Object.set("list", fgmethod(Object_list))

def List_map(self, args, env):
    if len(args.slots) > 2 or len(args.slots) == 0:
        raise FugaError, "`List map` expects 1 or 2 arguments."
    if len(args.slots) == 2:
        args = Object.clone((None,
            Method.clone(
                ('scope', env),
                (None, Object.clone((None, args.get(0)))),
                (None, args.get(1))
            )
        ))
    else:
        args = env.eval(args)

    if self.get('empty?') is fgtrue:
        return fglist()
    elif self.get('single?') is fgtrue:
        return fglist(
            args.get(0).call(self, 
                Object.clone((None, fgquote(self.get('value'))))
            , env)
        )
    elif self.get('conc?') is fgtrue:
        new_args = Object.clone((None, fgquote(args.get(0))))
        return List.clone(
            ('conc?', fgtrue),
            ('left',  List_map(self.get('left'),  new_args, env)),
            ('right', List_map(self.get('right'), new_args, env))
        )
    else:
        raise FugaError, "List map needs list instance."

List.set("map", fgmethod(List_map))

def List_conc(self, args, env):
    if len(args.slots) != 1:
        raise FugaError, "`List \++` requires 1 argument."
    args = env.eval(args)
    return List.clone(
        ('conc?', fgtrue),
        ('left',  self),
        ('right', args.get(0))
    )

def List_cons_right(self, args, env):
    if len(args.slots)!= 1:
        raise FugaError, "`List \<+ requires 1 argument."
    args = env.eval(args)
    return List.clone(
        ("conc?", fgtrue),
        ("left",  self),
        ("right", fglist(args.get(0))),
    )


List.set('++', fgmethod(List_conc))
List.set('<+', fgmethod(List_cons_right))


