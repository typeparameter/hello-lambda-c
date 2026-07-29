# Metadata
TARGET   = bootstrap
PLATFORM = linux/amd64
CC       = gcc
CFLAGS   = -std=c89 -pedantic -Wall -Iinclude

# Directories
SRCDIR   = src
BUILDDIR = build
OBJDIR   = $(BUILDDIR)/obj
BINDIR   = $(BUILDDIR)/bin

# Files
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: clean

$(TARGET): $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	@echo "Linking $@"
	@$(CC) $(OBJECTS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

zip: clean
	@echo "Building binary for platform $(PLATFORM)"
	@IMAGE=$$(docker build --platform=$(PLATFORM) -q .); \
		CONTAINER=$$(docker create --platform=$(PLATFORM) $$IMAGE); \
		docker cp -q $$CONTAINER:/workspace/build build; \
		docker rm $$CONTAINER > /dev/null
	@echo "Creating $(BUILDDIR)/lambda.zip"
	@zip -jq $(BUILDDIR)/lambda.zip build/bin/bootstrap

clean:
	rm -rf $(BUILDDIR)
	@echo "Project cleaned"
