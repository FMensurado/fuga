import parser
import values

def main():
    env = values.Object.clone()
    print "Fuga 0.0 parser. Type 'quit' to stop."
    print 
    while True:
        expr = raw_input(">>> ")
        if expr == 'quit': break
        ast  = parser.parse(expr)
        value = ast.convert()
        print repr(value)

if __name__ == '__main__':
    main()

