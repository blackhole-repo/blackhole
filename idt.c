// idt.c - Put this in your kernel
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;        // Kernel segment
    unsigned char  always0;
    unsigned char  flags;      // Present, Ring 0, etc.
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;
    // Load the IDT using assembly
    asm volatile("lidt (%0)" : : "r" (&idtp));
}