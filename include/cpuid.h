#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>


enum {
    IDX_EAX = 0,    // 0x0
    IDX_EBX = 1,    // 0x4
    IDX_EDX = 2,    // 0x8
    IDX_ECX = 3     // 0xC
};

typedef struct {
    uint32_t flags[4];
} cpuid_t;


extern int cpuid_valid(void) __asm__("cpuid_valid");
extern int cpuid_call(cpuid_t *buf) __asm__("cpuid_call");

const char *cpuid_vendor(void);


#endif