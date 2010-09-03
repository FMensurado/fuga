#/usr/bin/env python

from scanner import scanner
import tokens
import ast


def parse(code):
    """
    >>> parse('1000')
    number(1000)
    >>> parse('"xyz"lol')
    exp(string('xyz'), msg('lol'))
    >>> parse('()')
    object()
    >>> parse('(10)')
    object(number(10))
    >>> parse('(foo, bar)')
    object(msg('foo'), msg('bar'))
    >>> parse('(foo = bar, baz)')
    object(slot(msg('foo'), '=', msg('bar')), msg('baz'))
    >>> parse('fact(0) => 1, fact(n) => fact(n - 1) *(n)')
    object(slot(msg('fact', object(number(0))), '=>', number(1)), slot(msg('fact', object(msg('n'))), '=>', exp(msg('fact', object(exp(msg('n'), msg('-', number(1))))), msg('*', object(msg('n'))))))
    """
    return parse_tokens(scanner(code))

def parse_tokens(toks):
    """program ::= block* eof"""
    block = parse_block(toks)
    if toks.peek() == tokens.EOF:
        if len(block.slots) == 1:
            return block.slots[0]
        else:
            return block
    raise SyntaxError, "Expected EOF, got %s" % toks.peek()

def parse_block(toks):
    """block ::= (slot? sep)* slot? """
    slots = []
    while True:
        while toks.peek() == tokens.SEPARATOR:
            toks.next()
        if toks.peek() == tokens.EOF or toks.peek() == tokens.RPAREN:
            break
        slots.append(parse_slot(toks))
    return ast.block(slots)

def parse_slot(toks):
    """slot ::= (exp slotop)? exp"""
    exp1 = parse_exp(toks)
    if toks.peek() == tokens.EQUALS or toks.peek() == tokens.BECOMES:
        slotop = toks.next()
        exp2 = parse_exp(toks)
        if slotop == tokens.EQUALS:
            slotopstr = '='
        else:   
            slotopstr = '=>'
        return ast.slot(exp1, slotopstr, exp2)
    else:
        return exp1

def parse_exp(toks):
    """exp ::= root msg*"""
    root = parse_root(toks)
    msgs = []
    while toks.peek() == tokens.SYMBOL:
        msgs.append(parse_msg(toks))
    if not msgs:
        return root
    else:
        return ast.exp(root, msgs)
    

def parse_root(toks):
    """roor ::= atom | msg"""
    if toks.peek() == tokens.SYMBOL:
        return parse_msg(toks)
    else:
        return parse_atom(toks)

def parse_msg(toks):
    """msg ::= symbol args"""
    if not toks.peek() == tokens.SYMBOL:
        raise SyntaxError, "expected SYMBOL, got %s" % toks.peek()
    name = toks.next()
    if toks.peek() == tokens.LPAREN or toks.peek() == tokens.NUMBER or toks.peek() == tokens.STRING:
        atom = parse_atom(toks)
    else:
        atom = None
    return ast.msg(name.value, atom)

def parse_atom(toks):
    """atom ::= number | string | object
    """
    if toks.peek() == tokens.LPAREN:
        return parse_object(toks)
    elif toks.peek() == tokens.NUMBER:
        return ast.number(toks.next().value)
    elif toks.peek() == tokens.STRING:
        return ast.string(toks.next().value)
    else:
        raise SyntaxError, "expected an atom, got %s" % toks.peek()

def parse_object(toks):
    if not toks.peek() == tokens.LPAREN:
        raise SyntaxError, "expected LPAREN, got %s" % toks.peek()
    toks.next()
    block = parse_block(toks)
    if not toks.peek() == tokens.RPAREN:
        raise SyntaxError, "expected RPAREN, got %s" % toks.peek()
    toks.next()
    return block

if __name__ == '__main__':
    import doctest
    doctest.testmod()

