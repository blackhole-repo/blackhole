[extern handle_syscall]
global syscall_wrapper

syscall_wrapper:
    pushad              ; Save app's registers
    push eax            ; Push the syscall ID as an argument
    call handle_syscall
    add esp, 4          ; Clean up the stack
    popad               ; Restore registers
    iretd               ; Return to the app