gdt_start:
    dd 0x0 ; null descriptor
    dd 0x0

gdt_code: ; code segment descriptor
    dw 0xffff    ; Limit
    dw 0x0       ; Base
    db 0x0       ; Base 
    db 10011010b ; 1st flags, type flags
    db 11001111b ; 2nd flags, Limit
    db 0x0       ; Base

gdt_data: ; data segment descriptor
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start