#!/usr/bin/env python3

import sys
import prelude
from prelude.core import *
import parser

prelude.parse     = parser.parse
prelude.parseFile = parser.parseFile

def read(oldtext=''):
    while True:
        try:
            if oldtext:
                text = oldtext + input("... ")
            else:
                text = input(">>> ")
                if text.strip() in ['quit', 'quit()']:
                    return None
            return parser.parse(text)
        except parser.UnfinishedCode:
            oldtext = text + '\n'
        except SyntaxError as e:
            print(e)
        except EOFError:
            print("quit")
            return None

def repl():
    print("Prototypical Fuga interpreter, version 2.")
    env = prelude.Prelude.clone()
    while True:
        code = read()
        if code is None: break 

        try:
            code = code.thunk(env)
            code.thunkSlots(True, True)
        except FugaError as e:
            print("ERROR:", e)
    

        for slot in code:
            try:
                if not slot.is_(void):
                    print(slot)
            except FugaError as e:
                print("ERROR:", e)

def load(filename):
    try:
        code = parser.parseFile(filename)
        code.eval(prelude.Prelude, prelude.Prelude, True, True)
    except SyntaxError as e:
        print(e)
    except FugaError as e:
        print("ERROR:", e)

def usage():
    print("Usage:")
    print("\t%s [filename]" % sys.argv[0])

def main():
    if len(sys.argv) == 1:
        repl()
    elif len(sys.argv) == 2:
        load(sys.argv[1])
    else:
        usage()


if __name__ == '__main__':
    main()

