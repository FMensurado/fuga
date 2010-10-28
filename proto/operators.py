
std_groups = {
    # organizational operators
    0: ['->', '<-'],

    # comparison and boolean operators
    10: ['or', 'and'],
    11: ['==', '<', '>', '<=', '>='],

    # operators on aggregate types
    20: ['++'],
    21: ['<+', '+>'],

    # arithmetic, and other operators on atomic types
    30: ['+', '-'],
    31: ['*', '/', '%'],
    32: ['**'],

    # by default, operators are at 100
}

std_combines = {
    ':=': 'both',
    '+>': 'right',
}

def reverse(groups):
    """
    >>> reverse({})
    {}
    >>> reverse({'a': ['b']})
    {'b': 'a'}
    >>> reverse({'a': ['b', 'c']})
    {'c': 'a', 'b': 'a'}
    >>> reverse({'a': ['b'], 'c': ['d', 'e']})
    {'b': 'a', 'e': 'c', 'd': 'c'}
    >>> try: reverse({'a': ['b'], 'c': ['b']})
    ... except KeyError, e: print e
    'Duplicate operator definition: b'
    """
    result = {}
    for precedence in groups:
        for op in groups[precedence]:
            if op in result:
                raise KeyError, "Duplicate operator definition: %s" % op
            result[op] = precedence
    return result

def copy(groups):
    """
    >>> copy({})
    {}
    >>> a = {'a': ['b']}
    >>> b = copy(a)
    >>> b['foo'] = ['bar'] ; a ; b
    {'a': ['b']}
    {'a': ['b'], 'foo': ['bar']}
    >>> a = {'a': ['b']}
    >>> b = copy(a)
    >>> b['a'].append('c') ; a ; b
    {'a': ['b']}
    {'a': ['b', 'c']}
    """
    result = {}
    for precedence in groups:
        result[precedence] = groups[precedence][:]
    return result

def set_operator(groups, op, precedence):
    """
    >>> set_operator({}, '+', 60)
    {60: ['+']}
    >>> set_operator({1: ['+']}, '-', 2)
    {1: ['+'], 2: ['-']}
    >>> set_operator({1: ['+']}, '-', 1)
    {1: ['+', '-']}
    >>> set_operator({1: ['+', '-']}, '-', 2)
    {1: ['+'], 2: ['-']}
    >>> set_operator({1: ['+']}, '+', 60)
    {1: [], 60: ['+']}
    """
    if op in reverse(groups):
        del_operator(groups, op)
    if precedence in groups:
        groups[precedence].append(op)
    else:
        groups[precedence] = [op]
    return groups

def del_operator(groups, op):
    """
    >>> del_operator({10: ['+', '-']}, '+')
    {10: ['-']}
    >>> del_operator({10: ['-', '+']}, '+')
    {10: ['-']}
    >>> del_operator({10: ['+']}, '+')
    {10: []}
    """
    precedence = reverse(groups)[op]
    index = groups[precedence].index(op)
    del groups[precedence][index]
    return groups

def order(groups, ops):
    """
    >>> order(std_groups, [])
    []
    >>> order(std_groups, ['+'])
    [0]
    >>> order(std_groups, ['*', '+', '*'])
    [0, 1, 0]
    >>> order(std_groups, ['+', '-', '*'])
    [2, 0, 0]
    """
    # A O(n^2 log n) algorithm. Ugh.
    if len(ops) == 0:
        return []
    precedence_map  = reverse(groups)
    precedence_list = [(-precedence_map.get(op, 100),i) for i, op in enumerate(ops)]
    precedence_list.sort()
    index = precedence_list[0][1]
    ops = ops[:]
    del ops[index]
    return [index] + order(groups, ops) 


def solve(groups, values, ops, combine):
    """
    >>> def combine(l, op, r):
    ...     return '(' + l + op + r + ')'
    >>> solve(std_groups, ['a'], [], combine)
    'a'
    >>> solve(std_groups, ['a', 'b'], ['+'], combine)
    '(a+b)'
    >>> solve(std_groups, ['a', 'b', 'c'], ['+', '*'], combine)
    '(a+(b*c))'
    >>> solve(std_groups, ['a', 'b', 'c'], ['*', '+'], combine)
    '((a*b)+c)'
    >>> solve(std_groups, ['a', 'b', 'c'], ['+', '-'], combine)
    '((a+b)-c)'
    >>> solve(std_groups, ['a', 'b', 'c', 'd'], ['+', '==', '/'], combine)
    '((a+b)==(c/d))'
    """
    order_list = order(groups, ops)
    for op_index in order_list:
        op = ops[op_index]
        del ops[op_index]
        left  = values[op_index]
        right = values[op_index+1]
        del values[op_index+1]
        values[op_index] = combine(left, op, right)
    return values[0]

if __name__ == '__main__':
    import doctest
    doctest.testmod()

