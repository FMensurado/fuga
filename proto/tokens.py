
class token(object):
    def __init__(self, name, value=None, **kwargs):
        self.name = name
        self.value = value
        self.kwargs = kwargs
    
    def __repr__(self):
        extras = []
        if self.value is not None:
            extras.append(repr(self.value))
        for key in self.kwargs:
            extras.append("%s=%r" % (key, self.kwargs[key]))
        if extras:
            return "%s(%s)" % (self.name, ', '.join(extras))
        else:
            return self.name

    def __eq__(self, other):
        return self.name == other.name

    def __call__(self, value=None, **kwargs):
        return token(self.name, value, **kwargs)


STRING    = token("STRING")
NUMBER    = token("NUMBER")
SYMBOL    = token("SYMBOL")
LPAREN    = token("LPAREN")
RPAREN    = token("RPAREN")
EQUALS    = token("EQUALS")
BECOMES   = token("BECOMES")
SEPARATOR = token("SEPARATOR")
OPERATOR  = token("OPERATOR")
EOF       = token("EOF")
QUOTE     = token("QUOTE")
ESCAPE    = token("ESCAPE")

