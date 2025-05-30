extern tss
extern kmain

global long_mode_start

section .text
bits 64
long_mode_start:
    ; multiboot2 header
    pop rdi

    ; resetting segment registers for the future
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call kmain
    hlt

ok_status:
    ; green text BRUH
    mov rax, 0x2f482f552f522f42
    mov qword [0xb8000], rax