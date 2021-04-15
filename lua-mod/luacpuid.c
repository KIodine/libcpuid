#include <string.h>

#define LUA_LIB
#include "lua.h"
#include "lauxlib.h"

#include "cpuid.h"

/*
cpuid:
- params: eax, [ecx]
- return: eax, ebx, edx, ecx

Lua function call procedure:
- ...

Lua stack access:
- positive numbers [1, n] ascends from the bottom of stack.
- negative numbers [-1, -n] descends from the top of stack.
for example, `-1` represents the top of stack, `-2` refers to the
element right below the top, and so on, likewise the positive indices.

TODO:
- Lua struct: cpuid = {eax, ebx, edx, ecx}
    Struct as params also as return buffer.
- Implement `__tostring` metamethod in metatable.
    format: "<cpuid buffer [eax=0x%08x, ebx=0x%08x, edx=0x%08x, ecx=0x%08x]>"
- Find out how GC works in Lua.
*/

static const char *cpuid_mt = "cpuid_mt";

static const char *buf_fields[4] = {
    "eax",
    "ebx",
    "edx",
    "ecx"
};


static int _lua_cpuid_valid(lua_State *L){
    int valid = cpuid_valid();
    lua_pushboolean(L, valid);
    /* Only 1 return value. */
    return 1;
}

static int _cpu_vendor(lua_State *L){
    /* TODO */
    static const char *name;
    name = cpuid_vendor();
    lua_pushstring(L, name);
    return 1;
}


static int _buf_new(lua_State *L){
    int table_idx = -1;
    
    lua_createtable(L, 0, 4);
    /* set metatable */
    luaL_setmetatable(L, cpuid_mt);
    /* do init */
    for (int i = 0; i < 4; i++){
        lua_pushstring(L, buf_fields[i]);
        lua_pushinteger(L, (lua_Integer)0);
        table_idx -= 2;
    }
    for (int i = 0; i < 4; i++){
        lua_settable(L, table_idx);
        table_idx += 2;
    }
    return 1;
}

static int _extract_buf(lua_State *L, cpuid_t *buf){
    int table_idx = -1, res;
    
    /* Get each field from table, and check is integer */
    res = lua_istable(L, table_idx);
    if (!res){
        lua_pushstring(L, "Argument is not a table.");
        lua_error(L);   /* no return */
        return 0;
    }
    for (int i = 0; i < 4; i++){
        /* get fields in specific order: eax, ebx, edx, ecx. */
        lua_pushstring(L, buf_fields[i]);
        table_idx -= 1;
        lua_gettable(L, table_idx);
        res = lua_isinteger(L, -1);
        if (!res){
            lua_settop(L, table_idx-1);
            lua_pushfstring(L, "Argument %d is not an integer.", i);
            lua_error(L);   /* no return */
            return 0;
        }
        buf->flags[i] = (uint32_t)lua_tointeger(L, -1);
    }
    /* pop results. */
    lua_settop(L, table_idx);
}

static int _buf_tostr(lua_State *L){
    static const char *buffmt = "<cpuid buffer [eax=0x%08x, ebx=0x%08x, edx=0x%08x, ecx=0x%08x]>";
    cpuid_t buf;
    char strbuf[128];
    int table_idx = -1, res;
    
    _extract_buf(L, &buf);

    res = snprintf(
        strbuf, 128, buffmt,
        buf.flags[IDX_EAX], buf.flags[IDX_EBX],
        buf.flags[IDX_EDX], buf.flags[IDX_ECX]
    );
    strbuf[res] = '\0';
    
    lua_pushstring(L, strbuf);

    return 1;
}

static int _cpuid_call(lua_State *L){
    cpuid_t buf;
    int table_idx = -1;

    _extract_buf(L, &buf);
    
    cpuid_call(&buf);
    for (int i = (4-1); i >= 0; i--){
        /* push in reverse order */
        lua_pushinteger(L, (lua_Integer)buf.flags[i]);
        table_idx -= 1;
    }
    for (int i = 0; i < 4; i++){
        lua_setfield(L, table_idx, buf_fields[i]);
        table_idx += 1;
    }

    return 0;
}

static const luaL_Reg cpuid_methods[] = {
    {"New"          , _buf_new          },
    {"vendor"       , _cpu_vendor       },
    {"valid"        , _lua_cpuid_valid  },
    {"cpuid"        , _cpuid_call       },
    {NULL, NULL} /* sentinel */
};

// The `luaopen_*` is mandatory for lua to find init functions.
int luaopen_luacpuid(lua_State *L){
    /* create metatable */
    int mt_idx = -1;
    luaL_newmetatable(L, cpuid_mt);
    lua_pushcfunction(L, _buf_tostr);
    mt_idx -= 1;
    lua_setfield(L, mt_idx, "__tostring");
    lua_pop(L, 1);
    /* create table for methods */
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, cpuid_methods, 0);
    return 1;
}