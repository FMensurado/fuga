import parser
import values

def main():
    env = values.Object.clone()
    print "Fuga prototypical parser. Type 'quit' to stop."
    print 
    while True:
        try:
            expr = raw_input(">>> ")
        except EOFError:
            print
            break
        if expr == 'quit': break
        try:
            ast  = parser.parse(expr)
            value = ast.convert()
            evalue = env.eval(value)
            print repr(evalue)
        except SyntaxError, e:
            print "SYNTAX ERROR:", e
        except values.FugaError, e:
            print "ERROR:", e
    print

if __name__ == '__main__':
    main()

