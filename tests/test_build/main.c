#include "terminal.h"
#include "cursor.h"
#include "command.h"
#include "history.h"
#include "status.h"
#include "input.h"
#include "cleanup.h"
#include "mode.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <signal.h>
#include <stdlib.h>

// ==================== GLOBAL VARIABLES ====================
int running = 1;
// ==================== MAIN ====================
int main() 
{
    SetConsoleOutputCP(CP_UTF8);        // ensure it prints utf8 characters
    trm_save_original_color();          // save original consol colors
    trm_set_white_text();               // set current console color to white
    cleanup_setup();                    // setup initialization
    cursor_hide();                      // hide cursor
    while (running) {
        trm_redraw_border_if_changed(); // redraw border if terminal size changed
        // cmd_clear_message_if_expired(); // clear cli message when expired
        cursor_update_blink();		    // cursor blinking
        mode_display();                 // display the mode
        if (!input_handle_keyboard()) {
            break;                      // ESC pressed, break
        }
        Sleep(25);                      // sleep for 50 ms
    }
    cleanup();                          // exit cleanup code
    trm_gotoxy(0, term.height);
    printf("\n\nGoodbye!");             // print exit message
    return 0;                           // exit code success
}
