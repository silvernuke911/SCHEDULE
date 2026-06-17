#include "command.h"
#include "terminal.h"
#include "status.h"
#include "cursor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// --- command line and cursor
CommandLine cmdline = {{0}, 0, 0, 0};

// ==================== BUFFER FUNCTIONS ====================
void cmd_reset_buffer() {
    memset(cmdline.buffer, 0, sizeof(cmdline.buffer));
    cmdline.cursor_pos = 0;
	cmdline.scroll_offset = 0;
}

void cmd_clear_command_line() {
    trm_gotoxy(2 + term.prompt_len, term.height - 3);
    for (int i = 0; 
         i < (term.width - term.prompt_len - 13); 
         i++) 
    {
        printf(" ");
    }
    trm_gotoxy(2 + term.prompt_len, term.height - 3);
}

void cmd_redraw_buffer(void) {
    int y = term.height - 3;
    int visible_width =
        term.width - 14 - term.prompt_len;
    int len = strlen(cmdline.buffer);
    // keep cursor visible
    if (cmdline.cursor_pos < cmdline.scroll_offset)
    {
        cmdline.scroll_offset =
            cmdline.cursor_pos;
    }

    if (cmdline.cursor_pos >=
        cmdline.scroll_offset + visible_width)
    {
        cmdline.scroll_offset =
            cmdline.cursor_pos
            - visible_width + 1;
    }
    int max_offset = len - visible_width;
    if (max_offset < 0)
        max_offset = 0;
    if (cmdline.scroll_offset > max_offset)
        cmdline.scroll_offset = max_offset;
    trm_gotoxy(2 + term.prompt_len, y);

    for (int i = 0; i < visible_width; i++)
    {
        int idx = cmdline.scroll_offset + i;
        if (idx < len) {
            putchar(cmdline.buffer[idx]);
        } else {
            putchar(' '); 
        }
    }

    trm_gotoxy(
        2 + term.prompt_len +
        (cmdline.cursor_pos -
         cmdline.scroll_offset),
        y
    );

    fflush(stdout);
}

void cmd_add_char_to_buffer(char ch) {
    int len = strlen(cmdline.buffer);
	if (len >= CMD_BUF_SIZE - 1){
		status_set("ERR","Command line buffer maximum reached : max 256");
        return;
	}
    for (int i = len; i >= cmdline.cursor_pos; i--)
    {
        cmdline.buffer[i + 1] = cmdline.buffer[i];
    }

    cmdline.buffer[cmdline.cursor_pos] = ch;
    cmdline.cursor_pos++;

    cmd_redraw_buffer();
}

void cmd_remove_char_from_buffer(void) {
    int len = strlen(cmdline.buffer);
    if (cmdline.cursor_pos <= 0) return;

    // Check if buffer is currently full BEFORE removing
    int was_full = (len >= CMD_BUF_SIZE - 1);

    for 
    (
        int i = cmdline.cursor_pos - 1;
        i < len;
        i++
    ){
        cmdline.buffer[i] = cmdline.buffer[i + 1];
    }

    cmdline.cursor_pos--;
    cmd_redraw_buffer();
    // Only clear status if buffer was full and now isn't
    if (was_full && (len - 1 < CMD_BUF_SIZE - 1)) {
        status_clear();
    }  
}

// ==================== MESSAGE FUNCTIONS ====================
void cmd_show_message() 
{
    // Clear the command line first
    cmd_clear_command_line();
    
    // Show command executed
    status_set("CMD", cmdline.buffer);
    
    // Reset buffer
    cmd_reset_buffer();
    
    // Start message timer
    cmdline.showing_message = 1;
    cmdline.message_start_time = GetTickCount();
}

void cmd_clear_message_if_expired() 
{
    if (cmdline.showing_message && 
        (GetTickCount() - cmdline.message_start_time) >= 1000
    ){
		status_clear();
        // trm_draw_command_prompt();
        cmdline.showing_message = 0;
        // temporary;
        trm_clear_screen();
    }
}

// ==================== COMMAND PARSING ====================
ParsedCommand cmd_parse_command_buffer(){
    ParsedCommand cmd;
    
    cmd.count = 0;
    static char buffer_copy[CMD_BUF_SIZE];
    strcpy(buffer_copy, cmdline.buffer);
    char *token = strtok(buffer_copy, " ");

    while (token != NULL && cmd.count < MAX_TOKENS) {
        cmd.tokens[cmd.count] = token;
        cmd.count++;
        token = strtok(NULL, " ");
    }
    return cmd;
}

void cmd_process_commands(ParsedCommand cmd) {
    // sample text. we will use a switch statement to do shit later
    for (int i = 0; i< cmd.count; i++) {
        trm_gotoxy(5,5 + i);
        printf("%s",cmd.tokens[i]);
    }
}