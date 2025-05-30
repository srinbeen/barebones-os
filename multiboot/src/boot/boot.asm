extern long_mode_start

global gdt64
global tss

global kernel_code_selector
global tss_selector

global critical_stacks

global start

%define STACK_SIZE  4096
%define TSS_SIZE    0x68
%define PAGE_SIZE   4096

section .text
bits 32
start:
    ; stack pointer init
    mov esp, stack_top

    push 0
    push ebx

    ; error checking
    call check_multiboot
    call check_cpuid
    call check_long_mode

    ; set up paging
    call set_up_paging
    call enable_paging



    ; load the 64-bit GDT
    lgdt [gdt64.eogdt64]

    jmp gdt64_code_offset:long_mode_start


ok_status:
    ; print `OK` to screen
    ; 0xb8000: VGA buffer address
    mov dword [0xb8000], 0x2f4b2f4f ; little endian --> 0x2f white text green bkg  0x4f O 0x4b K
    hlt



; checks
check_multiboot:
    cmp eax, 0x36d76289 ; magic number loaded into eax when booted from multiboot
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error


error:
    ; print error code
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20 ; 0x4f white text red bkg `ERR:`
    mov byte  [0xb800a], al
    hlt



set_up_paging:
    mov eax, p3_table
    or eax, 0b11        ; present + writable
    mov [p4_table], eax
    
    mov eax, p2_table
    or eax, 0b11        ; present + writable
    mov [p3_table], eax

    mov ecx, 0          ; counter variable
.map_p2_table:
    mov eax, 0x200000               ; 2 MiB
    mul ecx                         ; eax *= ecx --> starting page address
    or eax, 0b10000011              ; present + writable + huge
    mov [p2_table + ecx * 8], eax   ; each 8-byte page entry has eax starting address

    inc ecx                         ; increase counter
    cmp ecx, 512                    ; all entries filled
    jne .map_p2_table               ; else map the next entry

    ret

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret


section .bss

; align to page size
align PAGE_SIZE

; 512 entries with 9-bit entry = 2^9 entries * 2^3 bytes per entry = 2^12 = 4096 bytes
p4_table:
    resb PAGE_SIZE
p3_table:
    resb PAGE_SIZE
p2_table:
    resb PAGE_SIZE

; STACKS
stack_bottom:
    resb STACK_SIZE
stack_top:

; stacks for critical ISRs
df_stack_bottom:
    resb STACK_SIZE
df_stack_top:

pf_stack_bottom:
    resb STACK_SIZE
pf_stack_top:

gp_stack_bottom:
    resb STACK_SIZE
gp_stack_top:

tss:
    resb TSS_SIZE


section .data
gdt64:
    dq 0 ; zero entry

.code:
    gdt64_code_offset   equ     $ - gdt64 
    ; 43: executable, 44: 1 for code segment, 47: present, 53: 64-bit segment
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
.tss:
    gdt64_tss_offset    equ     $ - gdt64
    dq 0
    dq 0

; End-of-GDT64
; GDT pointer data structure
.eogdt64:
    dw $ - gdt64 - 1    ; 4 byte (length - 1) 
    dq gdt64            ; 8-byte GDT start address


section .rodata

kernel_code_selector:
    dw gdt64_code_offset
tss_selector:
    dw gdt64_tss_offset

critical_stacks:
    dq stack_top
    dq df_stack_top
    dq pf_stack_top
    dq gp_stack_top