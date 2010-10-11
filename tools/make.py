#!/usr/bin/env python
"""make.py -- build fuga modules and packages



"""

CC = "gcc -Wall -Werror -std=c99 -pedantic"

def needs_building(module):
    omtime = os.path.getmtime(filename_o(module))
    cmtime = os.path.getmtime(filename_c(module))
    hmtime = os.path.getmtime(filename_h(module))
    return omtime < cmtime or omtime < hmtime

def include(module, path):
    """Calculate the new module."""

def deps_filename(module, filename, deps):
    file = open(filename)
    for line in file:
        if line[0] != "#": continue
        line = line[1:].strip()
        if line[:7] != "include": continue
        line = line[7:].lstrip()
        if line[0] != '"' and line[-1] != '"': continue
        dep = include(module, line[1:-1])
        if dep not in deps:
            deps.append(dep)
    

def deps(module, memo={}):
    if module in memo:
        return memo[module]
    deps = []
    deps_filename(module, filename_h(module), deps)
    deps_filename(module, filename_c(module), deps)
    memo[module] = deps
    return deps

def main():
    import sys
    print sys.argv

if __name__ == '__main__':
    import doctest
    doctest.testmod()
    main()
