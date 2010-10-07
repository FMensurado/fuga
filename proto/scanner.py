#!/usr/bin/env python

import tokens

class scanner(object):
    """
    >>> list(scanner(''))
    [EOF]
    >>> list(scanner('  \t  '))
    [EOF]
    >>> try: list(scanner('{'))
    ... except SyntaxError, e: print e
    unexpected character, '{', line 1 column 1
    >>> try: list(scanner('  } '))
    ... except SyntaxError, e: print e
    unexpected character, '}', line 1 column 3
    >>> list(scanner('('))
    [LPAREN(column=1, line=1), EOF]
    >>> list(scanner(')'))
    [RPAREN(column=1, line=1), EOF]
    >>> list(scanner('(= =>'))
    [LPAREN(column=1, line=1), EQUALS(column=2, line=1), BECOMES(column=4, line=1), EOF]
    >>> list(scanner('\\n='))
    [SEPARATOR(column=1, line=1), EQUALS(column=1, line=2), EOF]
    >>> list(scanner('hello'))
    [SYMBOL('hello', column=1, line=1), EOF]
    >>> list(scanner('hello world 2'))
    [SYMBOL('hello', column=1, line=1), SYMBOL('world', column=7, line=1), NUMBER(2, column=13, line=1), EOF]
    >>> list(scanner('"hello world!"'))
    [STRING('hello world!', column=1, line=1), EOF]
    >>> list(scanner('"h\\\\nell\o\\\\\\\\n\\\\""'))
    [STRING('h\\nell\\\\o\\\\n"', column=1, line=1), EOF]
    >>> list(scanner('"hi" there'))
    [STRING('hi', column=1, line=1), SYMBOL('there', column=6, line=1), EOF]
    >>> list(scanner('"hi\\nyou" there'))
    [STRING('hi\\nyou', column=1, line=1), SYMBOL('there', column=6, line=2), EOF]
    >>> list(scanner("foo=bar"))
    [SYMBOL('foo', column=1, line=1), EQUALS(column=4, line=1), SYMBOL('bar', column=5, line=1), EOF]
    >>> list(scanner("foo+>"))
    [SYMBOL('foo', column=1, line=1), OPERATOR('+>', column=4, line=1), EOF]
    >>> list(scanner("foor and or band"))
    [SYMBOL('foor', column=1, line=1), OPERATOR('and', column=6, line=1), OPERATOR('or', column=10, line=1), SYMBOL('band', column=13, line=1), EOF]
    >>> list(scanner("\\\\+"))
    [SYMBOL('+', column=1, line=1), EOF]
    >>> list(scanner('-10 +10 - 10 + 10'))
    [NUMBER(-10, column=1, line=1), NUMBER(10, column=5, line=1), OPERATOR('-', column=9, line=1), NUMBER(10, column=11, line=1), OPERATOR('+', column=14, line=1), NUMBER(10, column=16, line=1), EOF]
    >>> list(scanner("'foo`"))
    [QUOTE(column=1, line=1), SYMBOL('foo', column=2, line=1), ESCAPE(column=5, line=1), EOF]
    """
    def __init__(self, text, **kwargs):
        self.lineno = 1
        self.column = 1
        self.text   = text
        self.kwargs = kwargs
        self.token  = None
        self.done   = False

    def __iter__(self):
        return self

    def move_text(self, text):
        self.column += len(self.text) - len(text)
        self.text    = text 
    
    def strip(self):
        self.move_text(self.text.lstrip(' \t'))
        if self.text[:1] == '#':
            index = self.text.find('\n')
            self.move_text(self.text[index:])

    def scan(self):
        # strip leading whitespace and/or comment
        self.strip()

        # if we're done, we're done
        if not self.text:
            self.done = True
            return tokens.EOF

        # literal tokens are easy
        literal = {
            '(': tokens.LPAREN,
            ')': tokens.RPAREN,
            ',': tokens.SEPARATOR,
            '`': tokens.ESCAPE,
            "'": tokens.QUOTE
        }
        for key in literal:
            if self.text[:len(key)] == key:
                token = literal[key](line=self.lineno, column=self.column)
                self.move_text(self.text[1:])
                return token
        
        # newlines are a special case of literal token
        if self.text[0] == '\n':
            token = tokens.SEPARATOR(line=self.lineno, column=self.column)
            self.text = self.text[1:]
            self.lineno += 1
            self.column  = 1
            return token
        
        # get an entire word before deciding what it is
        alphabet = ('abcdefghijklmnopqrstuvwxyz'
                   +'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
                   +'0123456789')
        
        if self.text[0] in alphabet:
            alphabet += '?!'
            symbol = ''
            column = self.column
            while self.text and self.text[0] in alphabet:
                symbol += self.text[0]
                self.move_text(self.text[1:])
            
            # slot ops
            if symbol == 'and':
                return tokens.OPERATOR('and', line=self.lineno, column=column)
            if symbol == 'or':
                return tokens.OPERATOR('or',  line=self.lineno, column=column)
            
            # numbers
            try:
                val = int(symbol)
                return tokens.NUMBER(val, line=self.lineno, column=column)
            except ValueError:
                pass
            
            # symbols
            return tokens.SYMBOL(symbol, line=self.lineno, column=column)

        ###########
        alphabet = ('+-*/%^'
                   +'=><!'
                   +'~@$&*_;:|.?')
        if self.text[0] in alphabet:
            symbol = ''
            column = self.column
            while self.text and self.text[0] in alphabet:
                symbol += self.text[0]
                self.move_text(self.text[1:])

            # slot ops
            if symbol == '=':
                return tokens.EQUALS(line=self.lineno, column=column)
            if symbol == '=>':
                return tokens.BECOMES(line=self.lineno, column=column)

            # potential numbers
            if self.text and self.text[0] in '0123456789':
                if symbol == '+' or symbol == '-':
                    result = self.scan()
                    if result == tokens.NUMBER:
                        if symbol == '-':
                            result.value = -result.value
                        result.kwargs['column'] -= 1
                        return result
                    else:
                        raise SyntaxError, "ambiguous at %s, please add a space to differentiate between NUMBER SYMBOL and OPERATOR SYMBOL." % result

            # if there is an LPAREN right after, this is really a symbol
            if self.text and self.text[0] == '(':
                return tokens.SYMBOL(symbol,line=self.lineno,column=column)

            return tokens.OPERATOR(symbol,line=self.lineno, column=column)

        if self.text[0] == '\\':
            self.move_text(self.text[1:])
            token = self.scan()
            if token == tokens.OPERATOR:
                return tokens.SYMBOL(token.value,
                    column=token.kwargs['column']-1,
                    line=token.kwargs['line'])
            else:
                raise SyntaxError, "expected an OPERATOR, got %s" % token

        # now try strings
        token = self.scan_string()
        if token: return token

        # i am 12 and what is this i don't even
        raise SyntaxError, "unexpected character, %r, line %s column %s" % (self.text[0], self.lineno, self.column) 

    def scan_string(self):
        if self.text[0] != '"': return
        index = 1
        value = ''
        lineno = self.lineno
        column = self.column+1
        while index < len(self.text):
            if self.text[index] == '\\':
                index += 1; column += 1
                if index >= len(self.text): break
                replacements = {
                    'n': '\n',
                    't': '\t',
                    'r': '\r',
                    '"': '"',
                    '\\': '\\',
                    '\n': '',
                }
                if self.text[index] in replacements:
                    value += replacements[self.text[index]]
                else:
                    value += '\\' + self.text[index]
            elif self.text[index] == '"':
                token = tokens.STRING(value,
                    line=self.lineno, column=self.column)
                self.text = self.text[index+1:]
                self.column = column+1
                self.lineno = lineno
                return token
            else:
                value += self.text[index]
            
            if self.text[index] == '\n':
                lineno += 1
                column = 1
            else:
                column += 1

            index += 1
        raise SyntaxError, "mismatched quote, line %s column %s" % (
            self.lineno, self.column)

    def next(self):
        if self.done:
            raise StopIteration
        if self.token is None:
            return self.scan()
        else:
            token = self.token
            self.token = None
            return token

    def peek(self):
        if self.token is None:
            self.token = self.scan()
        return self.token

if __name__ == '__main__':
    import doctest
    failures = doctest.testmod()

