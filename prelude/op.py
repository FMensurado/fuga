from .prelude import *

operator = Prelude["operator"] = Object.clone()

# Highest Priority

operator['.'] = Object.clone(priority=fgint(-10), send=fgsym('scope'))

# Default Priority

operator['default'] = Object.clone(
    priority=fgint(0),
    send=fgsym('left')
)

# Arithmetic

operator['**']  = Object.clone(priority=fgint(50), send=fgsym('left'))
operator['*']  = Object.clone(priority=fgint(51), send=fgsym('left'))
operator['/']  = Object.clone(priority=fgint(51), send=fgsym('left'))
operator['//'] = Object.clone(priority=fgint(51), send=fgsym('left'))
operator['%']  = Object.clone(priority=fgint(51), send=fgsym('left'))
operator['+']  = Object.clone(priority=fgint(52), send=fgsym('left'))
operator['-']  = Object.clone(priority=fgint(52), send=fgsym('left'))

# Relational

operator['<']  = Object.clone(priority=fgint(60), send=fgsym('left'))
operator['>']  = Object.clone(priority=fgint(60), send=fgsym('left'))
operator['=='] = Object.clone(priority=fgint(60), send=fgsym('left'))
operator['<='] = Object.clone(priority=fgint(60), send=fgsym('left'))
operator['>='] = Object.clone(priority=fgint(60), send=fgsym('left'))

# Boolean

operator['&&'] = Object.clone(priority=fgint(70), send=fgsym('scope'))
operator['||'] = Object.clone(priority=fgint(70), send=fgsym('scope'))
operator['^^'] = Object.clone(priority=fgint(70), send=fgsym('scope'))

# Organization

operator['->'] = Object.clone(priority=fgint(100), send=fgsym('scope'))
operator['<-'] = Object.clone(priority=fgint(100), send=fgsym('scope'))

# Assignment

operator[':='] = Object.clone(priority=fgint(100), send=fgsym('scope'))

#############

def send(op):
    if op not in operator or 'send' not in operator[op]:
        op = 'default'
    return operator[op]['send'].value()

def priority(op):
    if op not in operator or 'priority' not in operator[op]:
        op = 'default'
    return operator[op]['priority'].value()

def combine(op, left, right):
    opsend = send(op)
    if opsend == 'left':
        return fgexpr(left, fgmsg(op, right)) 
    elif opsend == 'right':
        return fgexpr(right, fgmsg(op, left))
    elif opsend == 'scope':
        return fgmsg(op, left, right)
    raise TypeError("operator combine: don't understand 'send' option")

def maxpos(ops):
    bestpos = 0
    best    = priority(ops[0])
    for i,op in enumerate(ops):
        pri = priority(op)
        if pri < best:
            bestpos = i
            best = pri
    return bestpos

def shuffle(ops, parts):
    ops   = list(ops)
    parts = list(parts)
    while len(parts) > 1:
        i = maxpos(ops)
        parts[i] = combine(ops[i], parts[i], parts[i+1])
        del parts[i+1]
        del ops[i]
    return parts[0]

