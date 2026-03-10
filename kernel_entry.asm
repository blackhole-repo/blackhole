[bits 32]
[extern kernel_main] ; This matches the name in your kernel_main.c
call kernel_main     ; Jump to your C code
jmp $                ; If C returns, halt the CPU