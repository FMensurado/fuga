all: test fugai build

try: test fugai
	./fuga

test:
	tools/test fuga

fugai:
	tools/make --executable main && mv -f main fuga

debug:
	tools/make --test --executable main
	mv main debug_fuga
	gdb debug_fuga


build: fugai
	ar rcs bin/libfuga.a bin/fuga_*.o

install: build
	cp bin/libfuga.a /usr/lib
	rm -rf /usr/include/fuga
	mkdir /usr/include/fuga
	cp src/fuga/*.h /usr/include/fuga
	cp lib/*.fg /usr/include/fuga
	cp fuga /usr/bin

