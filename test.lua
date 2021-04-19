cpuid = require 'luacpuid'
print(cpuid.valid())
print(cpuid.vendor())
b = cpuid.New()
--print(b.eax)
print(b)
--b.eax = 0xe

cpuid.cpuid(b)
print(b)