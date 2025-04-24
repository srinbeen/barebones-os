extern irq_handler

%define DF 8     ; Double Fault
%define TS 10    ; Invalid TSS
%define NP 11    ; Segment Not Present
%define SS 12    ; Stack Segment Fault
%define GP 13    ; General Protection Fault
%define PF 14    ; Page Fault
%define AC 17    ; Alignment Check

%macro DEFINE_IRQ 2
irq%+%1:
    mov rsi, %1
    mov rdi, 34
    jmp %2   
%endmacro

.section text
bits 64

irq_stubs:
%assign i 0
%rep 256
%if (i = DF) || (i = TS) || (i = NP) || (i = SS) || (i = GP) || (i = PF) || (i = AC)
    DEFINE_IRQ i, irq_error_code
%else
    DEFINE_IRQ i, irq_common_stub
%endif
%assign i i+1
%endrep

irq_error_code:
    ; pop error codes and such
    nop
irq_common_stub:
    ; context stuff
    ; argument registers rdi, rsi, rdx, rcx, r8, r9
    call irq_handler
    iretq



section .rodata
global irq_stub_array

irq_stub_array:
%assign i 0
%rep 256
    dq irq%+i
%assign i i+1
%endrep