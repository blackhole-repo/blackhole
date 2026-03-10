// rxfs_driver.c - ENHANCED VERSION

#define RXFS_TABLE_SECTOR 17
#define MAX_FILES 16
#define SECTOR_SIZE 512

typedef struct {
    char name[12];           // Filename
    int start_sector;        // Where the file starts
    int file_size;           // Size in bytes
    int permissions;         // NEW: File permissions
    char reserved[4];        // Padding
} RxFS_Entry;

typedef struct {
    int fd;
    int file_index;
    int current_pos;
    bool is_open;
} FileDescriptor;

FileDescriptor fd_table[MAX_PROCESSES][8];
RxFS_Entry cached_table[MAX_FILES];
bool table_cached = false;

// Initialize file system
void rxfs_init() {
    // Read file table into cache
    read_disk_sector(RXFS_TABLE_SECTOR, (char*)cached_table);
    table_cached = true;
    
    // Initialize all file descriptors
    for (int i = 0; i < MAX_PROCESSES; i++) {
        for (int j = 0; j < 8; j++) {
            fd_table[i][j].is_open = false;
        }
    }
}

// Find file in RxFS
int find_file_in_rxfs(char* filename) {
    if (!table_cached) rxfs_init();
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(cached_table[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Open file - returns file descriptor
int sys_open_file(char* filename) {
    int file_index = find_file_in_rxfs(filename);
    if (file_index == -1) return -1;
    
    // Find free file descriptor
    for (int i = 0; i < 8; i++) {
        if (!fd_table[current_process_id][i].is_open) {
            fd_table[current_process_id][i].fd = i;
            fd_table[current_process_id][i].file_index = file_index;
            fd_table[current_process_id][i].current_pos = 0;
            fd_table[current_process_id][i].is_open = true;
            
            process_table[current_process_id].open_files[i] = i;
            return i;
        }
    }
    
    return -1; // No free descriptors
}

// Read from file
int sys_read_file(int fd, char* buffer, int size) {
    if (fd < 0 || fd >= 8) return -1;
    if (!fd_table[current_process_id][fd].is_open) return -1;
    
    FileDescriptor* file = &fd_table[current_process_id][fd];
    RxFS_Entry* entry = &cached_table[file->file_index];
    
    // Calculate how much we can read
    int remaining = entry->file_size - file->current_pos;
    if (size > remaining) size = remaining;
    if (size <= 0) return 0;
    
    // Read sectors
    int start_sector = entry->start_sector;
    int sector_offset = file->current_pos / SECTOR_SIZE;
    int byte_offset = file->current_pos % SECTOR_SIZE;
    
    char sector_buffer[SECTOR_SIZE];
    int bytes_read = 0;
    
    while (bytes_read < size) {
        read_disk_sector(start_sector + sector_offset, sector_buffer);
        
        int copy_size = SECTOR_SIZE - byte_offset;
        if (copy_size > size - bytes_read) {
            copy_size = size - bytes_read;
        }
        
        for (int i = 0; i < copy_size; i++) {
            buffer[bytes_read + i] = sector_buffer[byte_offset + i];
        }
        
        bytes_read += copy_size;
        file->current_pos += copy_size;
        sector_offset++;
        byte_offset = 0;
    }
    
    return bytes_read;
}

// Write to file
int sys_write_file(int fd, char* buffer, int size) {
    // Only SYSTEM permission processes can write
    if (process_table[current_process_id].permissions != PERM_SYSTEM) {
        return -1;
    }
    
    if (fd < 0 || fd >= 8) return -1;
    if (!fd_table[current_process_id][fd].is_open) return -1;
    
    FileDescriptor* file = &fd_table[current_process_id][fd];
    RxFS_Entry* entry = &cached_table[file->file_index];
    
    // Similar to read but with write_disk_sector
    // Implementation left as exercise
    
    return size;
}

// Close file
int sys_close_file(int fd) {
    if (fd < 0 || fd >= 8) return -1;
    if (!fd_table[current_process_id][fd].is_open) return -1;
    
    fd_table[current_process_id][fd].is_open = false;
    process_table[current_process_id].open_files[fd] = -1;
    
    return 0;
}

// Create new file
int sys_create_file(char* filename, int size_bytes) {
    if (process_table[current_process_id].permissions != PERM_SYSTEM) {
        return -1;
    }
    
    // Find empty entry in file table
    for (int i = 0; i < MAX_FILES; i++) {
        if (cached_table[i].name[0] == '\0') {
            // Copy filename
            int j;
            for (j = 0; j < 11 && filename[j] != '\0'; j++) {
                cached_table[i].name[j] = filename[j];
            }
            cached_table[i].name[j] = '\0';
            
            // Find free sectors (simplified - linear allocation)
            // In production, implement proper free space management
            cached_table[i].start_sector = find_free_sectors(size_bytes / SECTOR_SIZE + 1);
            cached_table[i].file_size = size_bytes;
            cached_table[i].permissions = 0644; // rw-r--r--
            
            // Write updated table back to disk
            write_disk_sector(RXFS_TABLE_SECTOR, (char*)cached_table);
            
            return i;
        }
    }
    
    return -1;
}

// Helper: Find contiguous free sectors
int find_free_sectors(int count) {
    // Simplified: just append after last file
    int max_sector = 18; // Start after file table
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (cached_table[i].name[0] != '\0') {
            int end = cached_table[i].start_sector + 
                     (cached_table[i].file_size / SECTOR_SIZE) + 1;
            if (end > max_sector) max_sector = end;
        }
    }
    
    return max_sector;
}