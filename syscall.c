// syscall.c - ENHANCED VERSION

#define MAX_SYSCALLS 32

// System call numbers
#define SYS_EXIT            0
#define SYS_MINIMIZE        1
#define SYS_CLOSE           2
#define SYS_READ_DISK       3
#define SYS_WRITE_DISK      4
#define SYS_OPEN_FILE       5
#define SYS_READ_FILE       6
#define SYS_WRITE_FILE      7
#define SYS_CLOSE_FILE      8
#define SYS_SEND_MSG        9
#define SYS_RECV_MSG        10
#define SYS_GET_PID         11
#define SYS_SLEEP           12
#define SYS_GET_TIME        13
#define SYS_ALLOC_MEMORY    14
#define SYS_FREE_MEMORY     15

// System call with multiple parameters
typedef struct {
    unsigned int eax;  // Syscall number
    unsigned int ebx;  // Param 1
    unsigned int ecx;  // Param 2
    unsigned int edx;  // Param 3
    unsigned int esi;  // Param 4
    unsigned int edi;  // Param 5
} SyscallParams;

// Return value structure
typedef struct {
    int return_value;
    int error_code;
} SyscallResult;

SyscallResult handle_syscall_ex(SyscallParams* params) {
    SyscallResult result = {0, 0};
    
    // Rate limiting: prevent syscall abuse
    process_table[current_process_id].syscall_count++;
    if (process_table[current_process_id].syscall_count > 1000) {
        // Too many syscalls in short time - possible attack
        result.error_code = -1;
        terminate_process(current_process_id);
        return result;
    }
    
    switch(params->eax) {
        case SYS_EXIT:
            result.return_value = params->ebx; // Exit code
            terminate_process(current_process_id);
            break;
            
        case SYS_MINIMIZE:
            manage_window(current_process_id, 0);
            break;
            
        case SYS_CLOSE:
            manage_window(current_process_id, 2);
            break;

        case SYS_READ_DISK:
            if (process_table[current_process_id].permissions == PERM_SYSTEM ||
                process_table[current_process_id].permissions == PERM_DRIVER) {
                int sector = params->ebx;
                char* buffer = (char*)params->ecx;
                read_disk_sector(sector, buffer);
                result.return_value = 0;
            } else {
                result.error_code = -2; // Permission denied
                log_security_event("Unauthorized disk read attempt");
            }
            break;
            
        case SYS_WRITE_DISK:
            if (process_table[current_process_id].permissions == PERM_SYSTEM) {
                int sector = params->ebx;
                char* buffer = (char*)params->ecx;
                write_disk_sector(sector, buffer);
                result.return_value = 0;
            } else {
                result.error_code = -2;
                log_security_event("Unauthorized disk write attempt");
            }
            break;
            
        case SYS_OPEN_FILE:
            result.return_value = sys_open_file((char*)params->ebx);
            break;
            
        case SYS_READ_FILE:
            result.return_value = sys_read_file(
                params->ebx,           // fd
                (char*)params->ecx,    // buffer
                params->edx            // size
            );
            break;
            
        case SYS_WRITE_FILE:
            result.return_value = sys_write_file(
                params->ebx,
                (char*)params->ecx,
                params->edx
            );
            break;
            
        case SYS_CLOSE_FILE:
            result.return_value = sys_close_file(params->ebx);
            break;
            
        case SYS_SEND_MSG:
            result.return_value = send_message(
                params->ebx,           // to_pid
                (char*)params->ecx,    // data
                params->edx            // size
            );
            break;
            
        case SYS_RECV_MSG:
            result.return_value = receive_message(
                (char*)params->ebx,    // buffer
                params->ecx            // max_size
            );
            break;
            
        case SYS_GET_PID:
            result.return_value = current_process_id;
            break;
            
        case SYS_SLEEP:
            sys_sleep(params->ebx);  // milliseconds
            break;
            
        case SYS_GET_TIME:
            result.return_value = (int)system_ticks;
            break;
            
        case SYS_ALLOC_MEMORY:
            result.return_value = (int)kmalloc(params->ebx);
            break;
            
        case SYS_FREE_MEMORY:
            kfree((void*)params->ebx);
            break;

        default:
            result.error_code = -3; // Unknown syscall
            break;
    }
    
    return result;
}

// Backward compatible wrapper
void handle_syscall(unsigned int syscall_id) {
    SyscallParams params = {syscall_id, 0, 0, 0, 0, 0};
    handle_syscall_ex(&params);
}