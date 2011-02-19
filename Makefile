all: test main

test:
	tools/test fuga

main::
	tools/make --executable --test main && mv main fuga

