#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VIDEO_MEMORY (char*)0xB8000

// Simple state machine for our "HTML" parser
void run_ppa(char* file_content) {
    int i = 0;
    int cursor_pos = 0;
    int color = 0x0F; // Default: White on Black

    while (file_content[i] != '\0') {
        // Look for tags
        if (file_content[i] == '<') {
            // Check for <h1> (Heading - Red)
            if (file_content[i+1] == 'h' && file_content[i+2] == '1') {
                color = 0x0C; // Light Red
                i += 4;       // Skip <h1>
            } 
            // Check for <p> (Paragraph - Gray)
            else if (file_content[i+1] == 'p') {
                color = 0x07; // Light Gray
                i += 3;       // Skip <p>
            }
            // Close tags
            else if (file_content[i+1] == '/') {
                color = 0x0F; // Reset to White
                while (file_content[i] != '>') i++;
                i++;
            }
        } else {
            // It's just normal text, print it to VGA memory
            VIDEO_MEMORY[cursor_pos * 2] = file_content[i];
            VIDEO_MEMORY[cursor_pos * 2 + 1] = color;
            cursor_pos++;
            i++;
        }

        // Handle screen overflow
        if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) break;
    }
}

void kernel_main() {
    // This would be your "app.ppa" content loaded from disk
    char* my_app = "<h1>PowerPC OS</h1><p>Running HTML5 app...</p>";
    
    run_ppa(my_app);
}