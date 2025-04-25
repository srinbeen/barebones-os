extern irq_handler

%define DF 8     ; Double Fault
%define TS 10    ; Invalid TSS
%define NP 11    ; Segment Not Present
%define SS 12    ; Stack Segment Fault
%define GP 13    ; General Protection Fault
%define PF 14    ; Page Fault
%define AC 17    ; Alignment Check

section .text
bits 64

; macro for irq_stubs
%macro DEFINE_IRQ 2
irq %+ %1:
%if %2 == 1
    push rax                ; put rax on stack to use rax as temp
    mov rax, rsi            ; old rsi in rax
    mov rsi, [rsp + 8]      ; RSI HAS ERROR CODE
    mov [rsp + 8], rax      ; write rax to stack where error code was
    pop rax                 ; restore rax
%else
    push rsi
%endif
    push rdi                ; SHOULD HAVE RSI AND RDI ON STACK
    mov rdi, %1             ; RDI HAS IRQ NUM
    jmp irq_common_stub   
%endmacro

irq_stubs:
%assign i 0
%rep 256
%if (i == DF) || (i == TS) || (i == NP) || (i == SS) || (i == GP) || (i == PF) || (i == AC)
    DEFINE_IRQ i, 1
%else
    DEFINE_IRQ i, 0
%endif
%assign i i+1
%endrep

irq_common_stub:
    ; stack:
    ; rsi
    ; rdi

    ; rdi currently has irq num, rsi currently has error or a random value

    ; context store
    push rax
    push rbx
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call irq_handler

    ; context restore
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rbx
    pop rax

    pop rdi
    pop rsi

    iretq


section .rodata
global irq_stub_array

irq_stub_array:
%assign i 0
%rep 256
    dq irq %+ i
%assign i i+1
%endrep