import parser
import values

def main():
    env = values.Object.clone()
    print "Fuga 0.0 parser. Type 'quit' to stop."
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
            # print repr(value)
            evalue = env.eval(value)
            print repr(evalue)
        except Exception, e:
            print "ERROR:", e
    print

if __name__ == '__main__':
    main()

