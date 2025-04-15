PUBLIC _spoofer_stub
     
.code
_spoofer_stub PROC
    mov r10, r10
    xchg r11, r11
    pop r11
    xchg rdi, rdi
    nop
    add rsp, 8
    xchg rax, rax
    jmp next
    nop
    xchg r10, r10
    nop

    next:
    xchg rax, rax
    mov rax, [rsp + 24]
    nop
    nop
    mov r10, [rax]
    xchg r11, r11
    mov [rsp], r10
    nop
    jmp next2
    mov r10, [rax + 8]
    nop
    
    next2:
    mov r10, [rax + 8]
    nop
    mov [rax + 8], r11
    xchg r15, r15
    mov [rax + 16], rdi
    xchg rdi, rdi
    lea rdi, fixup
    nop
    xchg rax, rax
    mov [rax], rdi
    jmp next3
    mov [rax + 16], rdi
    mov [rax + 8], r11
    nop

    next3:
    mov rdi, rax
    xchg rdi, rdi
    mov rdi, rdi
    xchg rdi, rdi
    mov r10, r10
    jmp r10
     
    fixup:
    xchg rdi, rdi
    sub rsp, 16
    xchg rcx, rcx
    mov rcx, rdi
    nop
    mov rdi, [rcx + 16]
    mov rcx, rcx
    jmp QWORD PTR [rcx + 8]
    _spoofer_stub ENDP
END