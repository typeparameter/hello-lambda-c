PREFIX = /usr/local
CFLAGS = -std=c89 -pedantic -Wall -Iinclude

.PHONY: lib install clean

lib: build/lib/liblambda.a

build/lib/liblambda.a: build/obj/http.o build/obj/runtime.o
	@mkdir -p build/lib
	@echo "Archiving $@"
	@$(AR) rcs $@ build/obj/http.o build/obj/runtime.o

build/obj/http.o: src/http.c src/http.h include/lambda.h
	@mkdir -p build/obj
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

build/obj/runtime.o: src/runtime.c src/http.h include/lambda.h
	@mkdir -p build/obj
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

install: lib
	@echo "Installing to $(DESTDIR)$(PREFIX)"
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig $(DESTDIR)$(PREFIX)/include
	@install -p -m 644 build/lib/liblambda.a $(DESTDIR)$(PREFIX)/lib
	@install -p -m 644 include/lambda.h $(DESTDIR)$(PREFIX)/include
	@sed 's|@PREFIX@|$(PREFIX)|' lambda.pc.in \
		> $(DESTDIR)$(PREFIX)/lib/pkgconfig/lambda.pc

clean:
	rm -rf build
	@echo "Project cleaned"
