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
    >>> parse('fact(0) => 1, fact(n) => fact(n - 1)*n')
    object(slot(msg('fact'), '=', msg('method', object(object(number(0)), number(1)))), slot(msg('fact'), '=', msg('method', object(object(msg('n')), opexp([msg('fact', object(opexp([msg('n'), number(1)], ['-']))), msg('n')], ['*'])))))
    >>> parse('a * b * c')
    opexp([msg('a'), msg('b'), msg('c')], ['*', '*'])
    >>> parse("'a")
    quote(msg('a'))
    >>> parse("foo `bar")
    exp(msg('foo'), msg('get', object(quote(msg('bar')))))
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
        if toks.peek() not in [tokens.SEPARATOR, tokens.EOF,tokens.RPAREN]:
            raise SyntaxError, "expected SEPARATOR, got %s" % toks.peek()
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
        return ast.slot(exp1, slotopstr, exp2).normalized()
    else:
        return exp1

def parse_exp(toks):
    """exp ::= part (op part)*"""
    parts = [parse_part(toks)]
    ops   = []
    while toks.peek() == tokens.OPERATOR:
        ops.append(toks.next().value)
        parts.append(parse_part(toks))
    if ops:
        return ast.opexp(parts, ops)
    else:
        return parts[0]

def parse_part(toks):
    """part ::= root msg*"""
    if toks.peek() == tokens.QUOTE:
        toks.next()
        return ast.quote(parse_part(toks))
    root = parse_root(toks)
    msgs = []
    while toks.peek() not in [tokens.EOF, tokens.SEPARATOR, tokens.RPAREN, tokens.OPERATOR, tokens.EQUALS, tokens.BECOMES]:
        msgs.append(parse_atom(toks))
    if not msgs:
        return root
    else:
        return ast.exp(root, msgs)
    

def parse_root(toks):
    """roor ::= atom | msg"""
    if toks.peek() == tokens.LPAREN:
        atom = parse_object(toks)
        if len(atom.slots) == 1 and isinstance(atom.slots[0], ast.opexp):
            return atom.slots[0]
        else:
            return atom
    else:
        return parse_atom(toks)

def parse_msg(toks):
    """msg ::= symbol args"""
    if toks.peek() == tokens.ESCAPE:
        toks.next()
        return ast.msg("get", ast.block([ast.quote(parse_msg(toks))]))
    if not toks.peek() == tokens.SYMBOL:
        raise SyntaxError, "expected SYMBOL, got %s" % toks.peek()
    name = toks.next()
    if toks.peek() == tokens.LPAREN:
        atom = parse_object(toks)
    else:
        atom = None
    return ast.msg(name.value, atom)

def parse_atom(toks):
    """atom ::= number | string | object
    """
    if toks.peek() == tokens.SYMBOL or toks.peek() == tokens.ESCAPE:
        return parse_msg(toks)
    elif toks.peek() == tokens.LPAREN:
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
    success = doctest.testmod()

