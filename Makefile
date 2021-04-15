AR := ar
CC := cc
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


.PHONY: static

$(OBJDIR):
	mkdir -p $(OBJDIR) $(BINDIR) $(LIBDIR)

$(SRCOBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c -o $@ $(CFLAGS) $^

$(ASMOBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.S | $(OBJDIR)
	$(CC) -c -o $@ $(CFLAGS) $^

$(LIBSTATIC): $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $^

static: $(LIBSTATIC)

$(LIBSHARED): CFLAGS+=-fPIC
$(LIBSHARED): $(LIBOBJS)
	$(CC) -o $@ $(CFLAGS) -shared $(LIBOBJS)
shared: $(LIBSHARED)

$(OBJDIR)/luacpuid.o: lua-mod/luacpuid.c
	$(CC) -c -o $@ $(CFLAGS) -I$(VNDDIR)/lua $^
# Adding `-fPIC` flag so can link against other object compiled with `-fPIC`.
# This will disable lots of warning by lua makefile.
# Also re-defines some flag override by custom flag.
LUA_FLAGS := -fPIC -std=c99 -DLUA_USE_LINUX -DLUA_USE_READLINE
$(VNDDIR)/lua/liblua.a:
	$(MAKE) -C $(dir $@) all MYCFLAGS="$(LUA_FLAGS)"


lib/luacpuid.so: CFLAGS += -fPIC
lib/luacpuid.so: $(OBJDIR)/luacpuid.o $(LIBSTATIC) $(VNDDIR)/lua/liblua.a
	$(CC) -o $@ $(CFLAGS) -shared -L$(VNDDIR)/lua $^
lua-mod: lib/luacpuid.so
#-L$(VNDDIR)/lua -llua

clean:
	rm -f $(LIBSTATIC) $(LIBSHARED) $(LIBLUAMOD) $(LIBOBJS)

# --- DO NOT MODIFY MANUALLY! ---
# > To update, use make command below:
depends:
	$(CC) -MM -I$(INCDIR) -I$(VNDDIR)/lua $(SRCDIR)/* lua-mod/*

# --- machine generated ---
cpuid-asm.o: src/cpuid-asm.S
cpuid.o: src/cpuid.c include/cpuid.h
luacpuid.o: lua-mod/luacpuid.c vendor/lua/lua.h vendor/lua/luaconf.h \
 vendor/lua/lauxlib.h vendor/lua/lua.h include/cpuid.h
