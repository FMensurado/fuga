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

def repl():
    env = values.Object.clone()
    print "Fuga prototypical interpreter. Type 'quit' to stop."
    print 
    while True:
        try:
            expr = raw_input(">>> ")
        except EOFError:
            print "quit"
            print "Goodbye."
            break
        if expr == 'quit':
            print "Goodbye."
            break

        try:
            ast    = parser.parse(expr)
            value  = ast.convert()
            evalue = env.eval(value)
            print repr(evalue)
        except SyntaxError, e:
            print "SYNTAX ERROR:", e
        except values.FugaError, e:
            print "ERROR:", e

def print_usage():
    print "Usage: %s [filename]" % sys.argv[0]

if __name__ == '__main__':
    if len(sys.argv) == 1:
        repl()
    elif len(sys.argv) == 2:
        run_file(sys.argv[1])
    else:
        print_usage()

