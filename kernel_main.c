// kernel_main.c
extern void idt_install();
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void timer_install(int hz);
extern void irq0_wrapper();
extern void syscall_wrapper();
extern int find_file_in_rxfs(char* filename);
extern void read_disk_sector(int lba, char* buffer);
extern void run_ppa(char* file_content);

void kernel_main() {
    // 1. Setup IDT and Hardware Interrupts
    idt_install();
    
    // Map IRQ0 (Timer) to IDT 32
    idt_set_gate(32, (unsigned long)irq0_wrapper, 0x08, 0x8E);
    // Map System Call (int 0x80)
    idt_set_gate(0x80, (unsigned long)syscall_wrapper, 0x08, 0x8E);
    
    timer_install(100); // 100Hz Heartbeat
    
    // 2. Enable Interrupts
    asm volatile("sti");

    // 3. App Loading Logic (Move buffer away from kernel at 0x1000)
    char* app_buffer = (char*)0x40000; 
    
    int sector = find_file_in_rxfs("APP.PPA");
    if (sector != -1) {
        read_disk_sector(sector, app_buffer);
        run_ppa(app_buffer);
    }
}