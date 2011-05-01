all: test main

try: all
	tools/try

test:
	tools/test fuga

main:
	tools/make --executable --test main && mv main fuga

