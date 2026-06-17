#include "history.h"
#include "terminal.h"
#include "command.h"
#include "cursor.h"
#include <string.h>
#include <stdio.h>

// --- command history
History history = {{{0}}, 0, 0};

// ==================== HISTORY FUNCTION MANAGEMENT ==========
void history_load_command() {
    cmd_clear_command_line();
    strcpy(
        cmdline.buffer, 
        history.history[history.index]
    );
    cmdline.cursor_pos = strlen(cmdline.buffer);
    
    // Calculate max display width (subtract prompt and margins)
    int max_display = term.width - 14 - term.prompt_len;
    int len = strlen(cmdline.buffer);
    
    // Determine starting position for scrolling
    int start = 0;
    if (len > max_display) {
        start = len - max_display;
    }
    
    // Print only the visible portion
    trm_gotoxy(2 + term.prompt_len, term.height - 3);
    int visible_len = (len - start < max_display) ? len - start : max_display;
    for (int i = 0; i < visible_len; i++) {
        printf("%c", cmdline.buffer[start + i]);
    }
    
    // Clear any remaining characters
    for (int i = visible_len; i < max_display; i++) {
        printf(" ");
    }
    
    // Position cursor
    int cursor_offset = cmdline.cursor_pos - start;
    trm_gotoxy(2 + term.prompt_len + cursor_offset, term.height - 3);
    cursor_draw_manual();
    fflush(stdout);
}