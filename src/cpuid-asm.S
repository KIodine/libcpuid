.intel_syntax noprefix

.text

.global cpuid_valid
.global cpuid_call

# ---
cpuid_valid:
    pushfq
    or  QWORD PTR [rsp], 0x200000
    popfq               # try set 21st bit.
    pushfq
    pop rdi
    and rdi, 0x200000   # test 21st bit is set.
    and rax, 0x0        # clear return
    cmp rdi, 0x0
    je  .invalid
    or  rax, 0x1
.invalid:
    ret

cpuid_call:
    mov rsi, rbx        # Don't forget to preserve registers!
    mov eax, DWORD PTR [rdi+0x0]
    mov ecx, DWORD PTR [rdi+0xC]
    cpuid
    mov DWORD PTR [rdi+0x0], eax
    mov DWORD PTR [rdi+0x4], ebx
    mov DWORD PTR [rdi+0x8], edx
    mov DWORD PTR [rdi+0xC], ecx
    mov rax, 0x0
    mov rbx, rsi        # recover rbx.
    ret
