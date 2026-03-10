[extern timer_handler]
global irq0_wrapper

irq0_wrapper:
    pushad          ; Save all registers (EAX, ECX, etc.)
    call timer_handler
    popad           ; Restore all registers
    iretd           ; Interrupt Return (32-bit)