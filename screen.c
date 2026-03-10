// screen.c
char virtual_buffer[80 * 25 * 2]; 

void flip_buffer() {
    char* video_mem = (char*)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i++) {
        video_mem[i] = virtual_buffer[i];
    }
}