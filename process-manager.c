#include <stdbool.h>

/* --- Definitions & Structures --- */
#define MAX_PROCESSES 10
#define STACK_SIZE 4096

// Permission Levels
typedef enum {
    PERM_USER = 0,
    PERM_SYSTEM = 1
} PermissionLevel;

// Process States
typedef enum {
    STATE_EMPTY,
    STATE_READY,
    STATE_RUNNING,
    STATE_MINIMIZED,
    STATE_KILLED
} ProcessState;

typedef struct {
    int pid;
    char name[32];
    ProcessState state;
    PermissionLevel permissions;
    unsigned int esp;          // Saved Stack Pointer
    unsigned int stack_base;   // Memory allocated for this app
    bool is_virus;
} PCB; // Process Control Block

/* --- Global OS State --- */
PCB process_table[MAX_PROCESSES];
int current_process_id = -1;

/* --- 1. The Antivirus (Static Scanner) --- */
// Scans the binary/HTML buffer for "illegal" byte patterns
bool antivirus_scan(char* app_code, int size) {
    for (int i = 0; i < size - 5; i++) {
        // Example "Malicious Signature": checking for code that 
        // attempts to overwrite the BIOS/Bootloader (fake signature)
        if (app_code[i] == 0xEB && app_code[i+1] == 0xFE) {
            return true; // Virus detected (Infinite Halt Loop)
        }
    }
    return false;
}

/* --- 2. Program Manager: Launching & Permissions --- */
int create_process(char* name, char* code, int size, PermissionLevel perms) {
    // A. Antivirus Check
    if (antivirus_scan(code, size)) {
        // Log security violation
        return -1; 
    }

    // B. Find empty slot
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == STATE_EMPTY) {
            process_table[i].pid = i;
            // Copy name logic here...
            process_table[i].permissions = perms;
            process_table[i].state = STATE_READY;
            
            // C. Setup Virtual Stack
            // In a real OS, you'd allocate memory here
            process_table[i].stack_base = (unsigned int)code; 
            return i;
        }
    }
    return -1;
}

/* --- 3. Window Manager: Minimize/Maximize/Close --- */
void manage_window(int pid, int action) {
    if (pid < 0 || pid >= MAX_PROCESSES) return;

    switch (action) {
        case 0: // Minimize
            process_table[pid].state = STATE_MINIMIZED;
            break;
        case 1: // Maximize / Restore
            process_table[pid].state = STATE_READY;
            break;
        case 2: // Close
            process_table[pid].state = STATE_KILLED;
            process_table[pid].state = STATE_EMPTY; // Free the slot
            break;
    }
}

/* --- 4. The Scheduler (Context Switcher) --- */
// This function would be called by the Timer Interrupt (IRQ 0)
void scheduler_tick() {
    // Find next process to run (Round Robin)
    int next_pid = (current_process_id + 1) % MAX_PROCESSES;
    
    while (process_table[next_pid].state != STATE_READY) {
        next_pid = (next_pid + 1) % MAX_PROCESSES;
        // If we loop back to start and find nothing, idle the CPU
        if (next_pid == current_process_id) return;
    }

    // Context Switch Logic:
    // 1. Save current registers to process_table[current_process_id].esp
    // 2. Update current_process_id = next_pid
    // 3. Load registers from process_table[next_pid].esp
    
    current_process_id = next_pid;
    process_table[current_process_id].state = STATE_RUNNING;
}

/* --- 5. Permission Enforcement (System Calls) --- */
void sys_write_disk(int sector, char* data) {
    // Security check: Only System apps can write to disk
    if (process_table[current_process_id].permissions != PERM_SYSTEM) {
        // App tried to do something it wasn't allowed to do!
        manage_window(current_process_id, 2); // Force Close (Kill)
        return;
    }
    // Proceed with disk write...
}