extern void flip_buffer(); // From screen.c

void timer_handler() {
    outb(0x20, 0x20); // End of Interrupt
    
    flip_buffer();    // Refresh the screen
    perform_context_switch(); // Run the Process Manager
}