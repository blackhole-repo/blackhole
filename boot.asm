[org 0x7c00]
KERNEL_OFFSET equ 0x1000 ; Memory offset to load our kernel

    mov [BOOT_DRIVE], dl ; BIOS stores boot drive in DL, save it
    mov bp, 0x9000       ; Set up the stack
    mov sp, bp

    call load_kernel     ; Load the C kernel from disk
    call switch_to_pm    ; Switch to Protected Mode (never returns)
    jmp $

%include "gdt.asm"       ; You need a GDT defined (see below)

[bits 16]
load_kernel:
    mov bx, KERNEL_OFFSET ; Destination
    mov dh, 15            ; Number of sectors to read
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02          ; Start reading from second sector
    int 0x13
    ret

switch_to_pm:
    cli                     ; 1. Disable interrupts
    lgdt [gdt_descriptor]   ; 2. Load Global Descriptor Table
    mov eax, cr0
    or eax, 0x1             ; 3. Set PE (Protection Enable) bit in CR0
    mov cr0, eax
    jmp CODE_SEG:init_pm    ; 4. Far jump to flush CPU pipeline

[bits 32]
init_pm:
    mov ax, DATA_SEG        ; 5. Update segment registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000        ; 6. Update stack for 32-bit
    mov esp, ebp

    call KERNEL_OFFSET      ; 7. JUMP TO YOUR C KERNEL!
    jmp $

BOOT_DRIVE db 0
