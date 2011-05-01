all: test main

try: all
	./fuga

test:
	tools/test fuga

main:
	tools/make --executable --test main && mv main fuga

