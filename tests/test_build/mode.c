#include "mode.h"
#include "terminal.h"

// ==================== GLOBAL VARIABLES ====================
int current_mode = HOME;
int previous_mode = -1;

void mode_display(void) {
    char *mode_string;
    int mode_width = 5;  // Width of the mode display
    if (current_mode == previous_mode) {
        return;
    }
    
    switch (current_mode) {
        case HOME:
            mode_string = "HOME ";
            break;
        case CALENDAR:
            mode_string = "CLNDR";
            break;
        case TASK:
            mode_string = "TASK ";
            break;
        case FOCUS:
            mode_string = "FOCUS";
            break;
        case LOCK:
            mode_string = "LOCK ";
            break;
        case CLOCK:
            mode_string = "CLOCK";
            break;
        case HISTORY:
            mode_string = "HSTRY";
            break; 
        default:
            mode_string = "?????";
            break;
    }
    
    // Position at right side of command line
    int x = term.width - mode_width - 6;  // 4 for "[ " and " ]"
    int y = term.height - 3;

    // Print the mode
    trm_gotoxy(x, y);
    printf("[ %s ]", mode_string);
    fflush(stdout);
    
    // Update previous_mode
    previous_mode = current_mode;
    return; 
} 