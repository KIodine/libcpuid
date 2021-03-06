AR := ar
CC ?= cc
AS := as

CFLAGS := -g -O0 -Wall -Wextra
ARFLAGS := rcs

PROJ_NAME := cpuid

INCDIR := include
SRCDIR := src
OBJDIR := objs
LIBDIR := lib
BINDIR := bin
VNDDIR := vendor

#$VENDOR_LIBDIR := lua
#VENDOR_LIBDIR := $(addprefix vendor/,$(VENDOR_LIBDIR))

#CFLAGS += $(addprefix -L,$(VENDOR_LIBDIR))

LIBNAME := lib$(PROJ_NAME)
LIBSTATIC := $(LIBDIR)/$(LIBNAME).a
LIBSHARED := $(LIBDIR)/$(LIBNAME).so
LIBLUAMOD := $(LIBDIR)/lua$(PROJ_NAME).so
LIBPYMOD := $(LIBDIR)/py$(PROJ_NAME).so # a `PYEXT` var for platform compat?

CFLAGS += $(addprefix -I,$(INCDIR))

SRCS := cpuid.c
SRCDST := $(addprefix $(SRCDIR)/,$(SRCS))
SRCOBJ := $(patsubst %.c,%.o,$(SRCS))
SRCOBJ := $(addprefix $(OBJDIR)/,$(SRCOBJ))

ASMS := cpuid-asm.S
ASMDST := $(addprefix $(SRCDIR)/,$(ASMS))
ASMOBJ := $(patsubst %.S,%.o,$(ASMS))
ASMOBJ := $(addprefix $(OBJDIR)/,$(ASMOBJ))

LIBOBJS := $(SRCOBJ) $(ASMOBJ)

TESTDIR := test
TESTS := test-cpuid.c
TESTDST := $(addprefix $(TESTDIR)/,$(TESTS))
TESTOBJ := $(patsubst %.c,%.o,$(TESTS))
TESTOBJ := $(addprefix $(OBJDIR)/,$(TESTOBJ))

TESTBIN := test-cpuid

LUADIR ?= $(VNDDIR)/lua
PYINCDIR ?= /usr/include/python3.7
PYLIBDIR ?= /usr/lib/python3.7/config-3.7m-x86_64-linux-gnu


.PHONY: static shared lua-mod py-mod clean depends testbin runtest

$(OBJDIR):
	mkdir -p $(OBJDIR) $(BINDIR) $(LIBDIR)

$(SRCOBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c -o $@ $(CFLAGS) $^

$(ASMOBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.S | $(OBJDIR)
	$(CC) -c -o $@ $(CFLAGS) $^

$(LIBSTATIC): $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $^

static: $(LIBSTATIC)

$(LIBSHARED): CFLAGS += -fPIC
$(LIBSHARED): $(LIBOBJS)
	$(CC) -o $@ $(CFLAGS) -shared $(LIBOBJS)
shared: $(LIBSHARED)

# --- test ---
$(TESTOBJ): $(OBJDIR)/%.o: $(TESTDIR)/%.c
	$(CC) -c -o $@ $(CFLAGS) $^
$(BINDIR)/$(TESTBIN): $(TESTOBJ) $(LIBSTATIC)
	$(CC) -o $@ $(CFLAGS) $^
testbin: $(BINDIR)/$(TESTBIN)

runtest: $(BINDIR)/$(TESTBIN)
	./$(BINDIR)/$(TESTBIN)

# --- Lua lib ---
$(OBJDIR)/luacpuid.o: lua-mod/luacpuid.c
	$(CC) -c -o $@ $(CFLAGS) -I$(LUADIR) $^
# Adding `-fPIC` flag so can link against other object compiled with `-fPIC`.
# This will disable lots of warning by lua makefile.
# Also re-defines some flag override by custom flag.
LUA_FLAGS := -fPIC -std=c99 -DLUA_USE_LINUX -DLUA_USE_READLINE
$(VNDDIR)/lua/liblua.a:
	$(MAKE) -C $(dir $@) all MYCFLAGS="$(LUA_FLAGS)"
	test -f ./lua || ln -s $(VNDDIR)/lua/lua .


lib/luacpuid.so: CFLAGS += -fPIC
lib/luacpuid.so: $(OBJDIR)/luacpuid.o $(LIBSTATIC) $(LUADIR)/liblua.a
	$(CC) -o $@ $(CFLAGS) -shared -L$(LUADIR) $^
lua-mod: lib/luacpuid.so

test-lua-mod: lib/luacpuid.so
	test -f luacpuid.so || ln -s lib/luacpuid.so
	./$(VNDDIR)/lua/lua ./test.lua


# --- Python lib ---
$(OBJDIR)/pycpuid.o: py-mod/pycpuid.c
	$(CC) -c -o $@ $(CFLAGS) -I$(PYINCDIR) $^
lib/pycpuid.so: CFLAGS += -fPIC
lib/pycpuid.so: $(OBJDIR)/pycpuid.o $(LIBSTATIC)
	$(CC) -o $@ $(CFLAGS) -shared -L$(PYLIBDIR) -lpython3.7 $^
py-mod: lib/pycpuid.so

test-py-mod: lib/pycpuid.so
	test -f pycpuid.so || ln -s lib/pycpuid.so .
	python3 ./modtest.py


# --- generic tools ---
clean:
	rm -f $(LIBSTATIC) $(LIBSHARED) $(LIBLUAMOD) $(LIBOBJS) \
 $(LIBPYMOD)
	rm -f ./lua ./*.so

# --- DO NOT MODIFY MANUALLY! ---
# > To update, use make command below:
depends:
	$(CC) -MM -I$(INCDIR) -I$(VNDDIR)/lua $(SRCDIR)/* lua-mod/* py-mod/*

# --- machine generated ---
cpuid-asm.o: src/cpuid-asm.S
cpuid.o: src/cpuid.c include/cpuid.h
luacpuid.o: lua-mod/luacpuid.c vendor/lua/lua.h vendor/lua/luaconf.h \
 vendor/lua/lauxlib.h vendor/lua/lua.h include/cpuid.h
pycpuid.o: py-mod/pycpuid.c include/cpuid.h
