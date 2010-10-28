import sys
import parser
import values

def run_code(code, env = None):
    if env is None:
        env = values.Object.clone()
    try:
        ast    = parser.parse(code)
        value  = ast.convert()
        evalue = env.eval(value)
        return evalue
    except SyntaxError, e:
        print "SYNTAX ERROR:", e
    except values.FugaError, e:
        print "ERROR:", e

def run_file(filename):
    code = open(filename).read()
    return run_code(code)

def read(pre_expr=''):
    # Read in the code
    try:
        if pre_expr:
            expr = pre_expr + raw_input("... ")
        else:
            expr = raw_input(">>> ")
    except EOFError:
        if pre_expr:
            print
        else:
            print "quit"
        print "Goodbye."
        sys.exit(0)

    if expr == 'quit':
        print "Goodbye."
        sys.exit(0)

    # Now parse it to completion.
    try:
        ast = parser.parse(expr)
        return ast.convert()
    except SyntaxError, e:
        if 'got EOF' in str(e):
            return read(expr+'\n')
        else:
            print "SYNTAX ERROR:", e

def repl():
    env = values.Object.clone()
    print "Fuga prototypical interpreter. Type 'quit' to stop."
    print 
    while True:
        parsed = read()
        if not parsed: continue
        for (name,value) in parsed.slots:
            try:
                evalue = env.eval(value)
                if name:
                    env.set(name, evalue)
                if evalue is not values.fgvoid:
                    print repr(evalue)
            except values.FugaError, e:
                print "ERROR:", e
                break


def print_usage():
    print "Usage: %s [filename]" % sys.argv[0]

if __name__ == '__main__':
    if len(sys.argv) == 1:
        repl()
    elif len(sys.argv) == 2:
        run_file(sys.argv[1])
    else:
        print_usage()

