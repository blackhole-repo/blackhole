; context_switch.asm - Full context switching implementation

[bits 32]
[extern save_current_esp]
[extern get_next_esp]
[extern current_process_id]

global perform_context_switch

perform_context_switch:
    ; Save current process state
    pushad                      ; Push all general purpose registers
    
    push esp                    ; Save current ESP
    call save_current_esp
    add esp, 4
    
    ; Get next process ESP
    call get_next_esp
    
    ; Check if valid ESP returned
    cmp eax, 0
    je .no_switch
    
    ; Switch to new stack
    mov esp, eax
    
    ; Restore new process state
    popad                       ; Pop all general purpose registers
    
    ret
    
.no_switch:
    popad
    ret