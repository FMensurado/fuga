#!/usr/bin/env python3

import prelude
from prelude.core import *
import parser

def read():
    while True:
        try:
            text = input(">>> ")
            if text.strip() in ['quit', 'quit()']:
                return None
            return parser.parse(text)
        except SyntaxError as e:
            print("SYNTAX ERROR:", e)
        except EOFError:
            print("quit")
            return None

def repl():
    env = prelude.Prelude.clone()

    while True:
        code = read()
        if code is None: break 
        try:
            code = code.eval(env)
            for slot in code:
                print(slot)
        except FugaError as e:
            print ("ERROR:", e)


def main():
    print("Prototypical Fuga interpreter, version 2.")
    repl()

if __name__ == '__main__':
    main()

