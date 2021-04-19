# CPUID
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
`libcpuid` is a simple wrapper of x86 instruction `CPUID` in C.

## Introduction
This project is a simple practice of:
- x86_64 assembly
- Lua C API
- Python C API

and does not have strict project structure (for now, and maybe forever).

This project is only tested on 64-bit intel processor, test before use!

## Quick start
To build `libcpuid`, just call:
```
make static|shared
```

There are just 3 API in `libcpuid`:
```C
enum {
    IDX_EAX = 0,
    IDX_EBX = 1,
    IDX_EDX = 2,
    IDX_ECX = 3
};

typedef struct {
    uint32_t flags[4];
} cpuid_t

/* Test instruction CPUID is available. */
int cpuid_valid(void);
/*  Call CPUID using buf as parameter, for detail of result, search for 
    "x86_64 CPUID" */
void cpuid_call(cpuid_t *buf);
/* A small routine for getting result of vendor name. */
const char *cpuid_vendor(void);
```

`libcpuid` does not depend on any external library.

## Test
```
make runtest
```

## Language extension
### Lua
To build the Lua language binding, you may fetch the references lua project
with git submodule:
```
git submodule init
git submodule update
```
then:
```
make lua-mod
```
Or if have existing lua source, just:
```
# In Lua source:
git checkout v5.4.0-patch

# In libcpuid:
make lua-mod LUADIR=<path/to/lua>
```
### Python
The python C extension requires at least Python3.7, if you're not using
apt to maintain package, make sure you set both `PYINCDIR` and `PYLIBDIR`
properly:
```
make py-mod [PYINCDIR=<path/to/headers> PYLIBDIR=<path/to/shared/lib>]
```

## Test language extension
Lua:
```
make test-lua-mod
```

Python:
```
make test-py-mod
```

## License
This library is distributed under MIT license.

## References
Lua:
- https://www.lua.org/manual/5.4/
  Though not quite clear about how to make a C module.
Python:
- https://docs.python.org/3/c-api/index.html
- https://docs.python.org/3/extending/newtypes_tutorial.html
