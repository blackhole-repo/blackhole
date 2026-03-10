#define ATA_DATA       0x1F0
#define ATA_SECTOR_CNT 0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE_SEL  0x1F6
#define ATA_COMMAND    0x1F7
#define ATA_STATUS     0x1F7

// Prototypes for functions in i-o.c
extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
// ata_disk_driver.c - Add write function

void write_disk_sector(int lba, char* buffer) {
    // 1. Select drive and send LBA
    outb(ATA_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    // 2. Send sector count
    outb(ATA_SECTOR_CNT, 1);
    
    // 3. Send LBA address
    outb(ATA_LBA_LOW,  (unsigned char)lba);
    outb(ATA_LBA_MID,  (unsigned char)(lba >> 8));
    outb(ATA_LBA_HIGH, (unsigned char)(lba >> 16));
    
    // 4. Send WRITE command (0x30)
    outb(ATA_COMMAND, 0x30);

    // 5. Wait for drive to be ready
    while (!(inb(ATA_STATUS) & 0x08));

    // 6. Write the data (256 words = 512 bytes)
    unsigned short* ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA, ptr[i]);
    }
    
    // 7. Flush cache
    outb(ATA_COMMAND, 0xE7);
    while (inb(ATA_STATUS) & 0x80); // Wait for BSY to clear
}

// Output a word (2 bytes) to a port
static inline void outw(unsigned short port, unsigned short val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
void read_disk_sector(int lba, char* buffer) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20); // READ command

    while (!(inb(0x1F7) & 0x08)); // Wait for buffer

    unsigned short* ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }
}
}