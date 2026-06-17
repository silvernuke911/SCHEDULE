#ifndef COMMAND_H
#define COMMAND_H

#include <windows.h>

#define CMD_BUF_SIZE 256
#define MAX_HISTORY 256
#define MAX_TOKENS 64

// ==================== STRUCT DEFINITIONS ====================
typedef struct {
    char buffer[CMD_BUF_SIZE];
    int scroll_offset;
    int cursor_pos;
    int showing_message;
    DWORD message_start_time;
} CommandLine;

// --- command struct
typedef struct {
    char *tokens[MAX_TOKENS];
    int count;
} ParsedCommand;

// ==================== GLOBAL VARIABLES ====================
extern CommandLine cmdline;

// ==================== BUFFER FUNCTIONS ====================
void cmd_reset_buffer();
void cmd_clear_command_line();
void cmd_redraw_buffer(void);
void cmd_add_char_to_buffer(char ch);
void cmd_remove_char_from_buffer(void);

// ==================== MESSAGE FUNCTIONS ====================
void cmd_show_message();
void cmd_clear_message_if_expired();

// ==================== COMMAND PARSING ====================
ParsedCommand cmd_parse_command_buffer();
void cmd_process_commands(ParsedCommand cmd);

#endif