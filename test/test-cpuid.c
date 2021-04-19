#include <stdio.h>
#include <stdlib.h>

#include "cpuid.h"


#define NL "\n"


int main(void){
    int enabled;
    const char *name;
    cpuid_t buf = {};

    enabled = cpuid_valid();
    if (enabled){
        printf("CPUID is valid"NL);
    } else {
        fprintf(stderr, "Cannot test cpuid"NL);
    }
    name = cpuid_vendor();
    
    buf.flags[IDX_EAX] = 0;
    cpuid_call(&buf);

    printf("vendor is \"%s\", max=0x%08x"NL, name, buf.flags[IDX_EAX]);
    return 0;
}