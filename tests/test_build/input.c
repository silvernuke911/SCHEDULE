#include "input.h"
#include "terminal.h"
#include "command.h"
#include "history.h"
#include "cursor.h"
#include "status.h"
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ==================== INPUT HANDLING ====================
int input_handle_keyboard() 
{
    if (!_kbhit()) return 1;  // No key pressed, continue running
    
    int ch = _getch();
    // Check if it's an extended key first
    if (ch == 224 || ch == 0) {
        return input_handle_extended_keys(ch);
    }
    // Handle regular keys
    return input_handle_regular_keys(ch);
}

int input_handle_regular_keys(int ch)
{
    switch (ch) {
        case '\r':  // Enter
            input_handle_enter();
            break;
        case '\b':  // Backspace
            cmd_remove_char_from_buffer();
            break;
        case 27:  // ESC
            return 0;  // Exit program
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 ) {
                cmd_add_char_to_buffer(ch);
            }
            break;
    }
    return 1;  // Continue running
}

int input_handle_extended_keys(int ch)
{
    ch = _getch();
    
    switch (ch) {
        case 72: 
            handle_up_arrow(); 
            break;
        case 80: 
            handle_down_arrow(); 
            break;
        case 75: 
            handle_left_arrow(); 
            break;
        case 77: 
            handle_right_arrow(); 
            break;
        default: break;
    }
    return 1;
}

static void handle_up_arrow(void) {
    status_set("ARW", "^");
    if (history.count <= 0 || history.index <= 0) return;
    cursor_erase_old();
    history.index--;
    history_load_command();
}

static void handle_down_arrow(void) {
    status_set("ARW", "_");
    if (history.index >= history.count - 1) {
        history.index = history.count;
        cmd_clear_command_line();
        cmd_reset_buffer();
        return;
    }
    cursor_erase_old();
    history.index++;
    history_load_command();
}

static void handle_left_arrow(void) {
    status_set("ARW", "<");
    if (cmdline.cursor_pos <= 0) return;
    cmdline.cursor_pos--;
    trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
}

static void handle_right_arrow(void) {
    status_set("ARW", ">");
    if (cmdline.cursor_pos >= (int)strlen(cmdline.buffer)) return;
    cmdline.cursor_pos++;
    trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
}

void input_handle_enter() 
{
    if (strlen(cmdline.buffer) > 0) 
    {   // Only show if there's a command
        ParsedCommand cmd = cmd_parse_command_buffer();
        cmd_process_commands(cmd);
        input_add_history();
        cmd_show_message();
    } else {
        // Empty command - just clear the line and reset
        cmd_clear_command_line();
        cmd_reset_buffer();
        status_clear();
    }
}

void input_add_history(void) 
{
    if (history.count >= MAX_HISTORY) {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(history.history[i-1], history.history[i]);
        }
        history.count = MAX_HISTORY - 1;
    }

    strcpy(history.history[history.count], cmdline.buffer);
    history.count++;
    history.index = history.count;
    return;
}