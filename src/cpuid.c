#include "cpuid.h"

/* refs:
- https://www.cs.virginia.edu/~evans/cs216/guides/x86.html
- https://www.felixcloutier.com/x86/cpuid
- https://www.felixcloutier.com/x86/pushf:pushfd:pushfq
- https://en.wikipedia.org/wiki/FLAGS_register
- https://www.felixcloutier.com/x86/pop
*/


static inline __attribute__((unused))
uint32_t bitrev32(uint32_t dw){
    dw = ((dw & 0x55555555) <<  1)|((dw & 0xaaaaaaaa) >>  1);
    dw = ((dw & 0x33333333) <<  2)|((dw & 0xcccccccc) >>  2);
    dw = ((dw & 0x0f0f0f0f) <<  4)|((dw & 0xf0f0f0f0) >>  4);
    dw = ((dw & 0x00ff00ff) <<  8)|((dw & 0xff00ff00) >>  8);
    dw = ((dw & 0x0000ffff) << 16)|((dw & 0xffff0000) >> 16);
    return dw;
}

// or using external buffer?
const char *cpuid_vendor(void){

    static union {
        char name[16];
        uint32_t u32_view[3];
    } un;

    cpuid_t buf = {};
    
    buf.flags[IDX_EAX] = 0;
    cpuid_call(&buf);
    un.u32_view[0] = buf.flags[IDX_EBX];
    un.u32_view[1] = buf.flags[IDX_EDX];
    un.u32_view[2] = buf.flags[IDX_ECX];
    return un.name;
}