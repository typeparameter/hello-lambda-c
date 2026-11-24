# Metadata
IMAGE_BUILDER   = docker
TARGET_PLATFORM = linux/amd64
PREFIX          = /usr/local
CC              = gcc
AR              = ar
CFLAGS          = -std=c89 -pedantic -Wall -Iinclude

.PHONY: lib example install zip clean

lib: build/lib/liblambda.a

example: build/bin/bootstrap

# Library
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

# Example
build/bin/bootstrap: build/obj/example/main.o build/lib/liblambda.a
	@mkdir -p build/bin
	@echo "Linking $@"
	@$(CC) build/obj/example/main.o -Lbuild/lib -llambda -o $@

build/obj/example/main.o: examples/hello/main.c include/lambda.h
	@mkdir -p build/obj/example
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

install: lib
	@echo "Installing to $(DESTDIR)$(PREFIX)"
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig $(DESTDIR)$(PREFIX)/include
	@install -p -m 644 build/lib/liblambda.a $(DESTDIR)$(PREFIX)/lib
	@install -p -m 644 include/lambda.h $(DESTDIR)$(PREFIX)/include
	@sed 's|@PREFIX@|$(PREFIX)|' lambda.pc.in \
		> $(DESTDIR)$(PREFIX)/lib/pkgconfig/lambda.pc

zip: clean
	@echo "Building binary for platform $(TARGET_PLATFORM)"
	@$(IMAGE_BUILDER) build \
		--platform $(TARGET_PLATFORM) \
		--target artifact \
		--output type=local,platform-split=false,dest=build \
		.
	@echo "Creating build/lambda.zip"
	@zip -jq build/lambda.zip build/bin/bootstrap

clean:
	rm -rf build
	@echo "Project cleaned"
