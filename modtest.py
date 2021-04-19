import typing

import pycpuid


assert pycpuid.CPUID_VALID
print(pycpuid.CPUID_VENDOR)

buf = pycpuid.Buffer()
print(buf)
#print(buf.eax)
buf.eax = 0x0

print(pycpuid.Cpuid)
try:
    pycpuid.Cpuid(buf)
except Exception:
    pass

try:
    pycpuid.Cpuid(123)
except TypeError as e:
    print(f"Raise expected: {e}")

print(buf)
del buf
print("exit")
