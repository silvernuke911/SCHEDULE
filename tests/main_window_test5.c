#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define CMD_BUF_SIZE 256
#define MAX_HISTORY 256
#define MAX_TOKENS 64

// ==================== STRUCT DEFINITIONS ====================
typedef struct {
    int width;
    int height;
    int old_width;
    int old_height;
    int sep_bar;
	int prompt_len;
} Terminal;

typedef struct {
    int x;
    int y;
    int visible;
    int prev_pos;
    DWORD last_blink;
} Cursor;

typedef struct {
    char buffer[CMD_BUF_SIZE];
    int scroll_offset;
    int cursor_pos;
    int showing_message;
    DWORD message_start_time;
} CommandLine;

typedef struct {
    char history[MAX_HISTORY][CMD_BUF_SIZE];
    int count;
    int index;
} History;

// ==================== MODE ENUMS ==================
typedef enum {
	HOME = 0,
	CALENDAR,
	TASK,
	FOCUS
} MODE;

// ==================== COMMAND ENUMS ====================
// Main commands
typedef enum {
    CMD_NONE = 0,
    CMD_HOME,
    CMD_HELP,
    CMD_CLOCK,
    CMD_CALENDAR,
    CMD_TASK,
    CMD_FOCUS,
    CMD_EXIT,
    CMD_HISTORY,
    CMD_LOCK,
    CMD_CLEAR
} Command;

// Clock subcommands
typedef enum {
    CLOCK_NONE = 0,
    CLOCK_SET_TIME,
    CLOCK_SET_DATE
} ClockSubcommand;

// Calendar subcommands
typedef enum {
    CAL_NONE = 0,
    CAL_MONTH,
    CAL_YEAR,
    CAL_WEEK,
    CAL_DAY
} CalendarSubcommand;

// Task subcommands
typedef enum {
    TASK_NONE = 0,
    TASK_TODO,
    TASK_OVERDUE,
    TASK_DONE,
    TASK_ADD,
    TASK_REMOVE,
    TASK_EDIT,
    TASK_ALL,
    TASK_SORT,
    TASK_SETTING
} TaskSubcommand;

// Task sort types - FIXED: renamed to avoid Windows macro conflict
typedef enum {
    TASK_SORT_DEFAULT = 0,
    TASK_SORT_CREATION,
    TASK_SORT_PRIORITY,
    TASK_SORT_DEADLINE,
    TASK_SORT_STATUS
} TaskSortType;

// Focus subcommands
typedef enum {
    FOCUS_NONE = 0,
    FOCUS_SET_N,
    FOCUS_SET_ID
} FocusSubcommand;
// ==================== COMMAND STRUCT ====================
typedef struct {
    Command cmd;
    int subcmd;
    char *args[MAX_TOKENS];
    int arg_count;
} ParsedCommandEx;
// --- command struct
typedef struct {
    char *tokens[MAX_TOKENS];
    int count;
} ParsedCommand;
// ==================== GLOBAL VARIABLES ====================
// --- terminal width
Terminal term = {0, 0, -1, -1, 4, 8};
// --- state
int running = 1;
// --- command line and cursor
Cursor cursor = {0, 0, 1, 0, 0};
CommandLine cmdline = {{0}, 0, 0, 0};
// --- command history
History history = {{{0}}, 0, 0};

MODE current_mode = HOME;

// Forward declarations
Command parse_command_string(const char *str);
int validate_command(ParsedCommandEx *cmd);
int process_command(ParsedCommandEx *cmd, const char *original_command);
void show_help(void);
void show_current_time(void);
void show_history(void);
//===================== TERMINAL COLORS =====================

WORD original_color = 0;  // Store original color globally

void trm_save_original_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    original_color = csbi.wAttributes;
}

void trm_set_console_color(int foreground, int background) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (background * 16) + foreground);
}

void trm_restore_original_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, original_color);
}


void trm_set_white_text() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);  // 7 = white text on black background
}

// ==================== TERMINAL FUNCTIONS ====================
void get_terminal_size(int *width, int *height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void trm_gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ===================== STATUS LINE ========================
void status_clear(void) {
    trm_gotoxy(2, term.height - 2);
    for (int i = 0; i < term.width - 4; i++)
        putchar(' ');
    fflush(stdout);
}

void status_draw(const char *msg) {
    trm_gotoxy(2, term.height - 2);
    int max_len = term.width - 4;
    for (int i = 0; i < max_len; i++)
    {
        char c = msg[i];
        if (c == '\0')
            break;
        putchar(c);
    }
    // optional: clear remainder of line if message is shorter
    for (int i = strlen(msg); i < max_len; i++)
        putchar(' ');
    fflush(stdout);
}

void status_set(const char *tag, const char *msg) {
    char buf[512];
    snprintf(buf, sizeof(buf), "[ %s ]: %s", tag, msg);
    status_draw(buf);
}
// ==================== CURSOR FUNCTIONS ====================
void cursor_show() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void cursor_hide() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void cursor_erase_old() {
    int screen_pos =
        cursor.prev_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;

    trm_gotoxy(x, y);

    if (cursor.prev_pos < strlen(cmdline.buffer))
        putchar(cmdline.buffer[cursor.prev_pos]);
    else
        putchar(' ');
}

void cursor_draw_manual() {
    int screen_pos =
        cmdline.cursor_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int visible_width = term.width - 4 - term.prompt_len;

    if (screen_pos >= visible_width) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;
    trm_gotoxy(x, y);
    putchar('_');
    cursor.prev_pos = cmdline.cursor_pos;
    fflush(stdout);
}

void cursor_update_blink() {
    DWORD now = GetTickCount();
    if (now - cursor.last_blink >= 500) {
        cursor.last_blink = now;
        if (cursor.visible) {
            // hide cursor
            cursor_erase_old();
            cursor.visible = 0;
        }
        else {
            // show cursor
            cursor_draw_manual();
            cursor.visible = 1;
        }
    }
}


// ==================== BUFFER FUNCTIONS ====================
void cmd_reset_buffer() {
    memset(cmdline.buffer, 0, sizeof(cmdline.buffer));
    cmdline.cursor_pos = 0;
	cmdline.scroll_offset = 0;
}

void cmd_clear_command_line() {
    trm_gotoxy(2 + term.prompt_len, term.height - 3);
    for (int i = 0; i < (term.width - 13 - term.prompt_len); i++) {
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
		status_set("ERR","command line buffer maximum reached");
        return;
	}
    for (int i = len; i >= cmdline.cursor_pos; i--)
    {
        cmdline.buffer[i + 1] =
            cmdline.buffer[i];
    }

    cmdline.buffer[cmdline.cursor_pos] = ch;
    cmdline.cursor_pos++;

    cmd_redraw_buffer();
}

void cmd_remove_char_from_buffer(void) {
    int len = strlen(cmdline.buffer);
    if (cmdline.cursor_pos <= 0)
        return;

    for (int i = cmdline.cursor_pos - 1;
         i < len;
         i++)
    {
        cmdline.buffer[i] =
            cmdline.buffer[i + 1];
    }

    cmdline.cursor_pos--;
	if (len < CMD_BUF_SIZE - 1) status_clear();
    cmd_redraw_buffer();
}

// ==================== DRAWING FUNCTIONS ====================
void trm_clear_screen() {
    for (int x = 1; x < term.width - 1; x++) {
        for (int y = 1; y < term.height - term.sep_bar; y ++){
            trm_gotoxy(x,y);
            printf(" ");
        }
    }
}

void trm_draw_border(int width, int height, int sep_bar) {
    // Top border (y = 0)
    trm_gotoxy(0, 0);
    printf("╔");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╗");
    
    // Bottom border (y = height - 1)
    trm_gotoxy(0, height - 1);
    printf("╚");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╝");
    
    // Separator bar (y = height - sep_bar)
    trm_gotoxy(0, height - sep_bar);
    printf("╠");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╣");
    
    // Left and right borders
    for (int y = 1; y < height - 1; y++) {
        if (y == height - sep_bar) continue;  // Already drawn
        
        trm_gotoxy(0, y);
        printf("║");
        
        trm_gotoxy(width - 1, y);
        printf("║");
    }
}

void trm_draw_command_prompt() {
    int y = term.height - 3;
    trm_gotoxy(2, y);
    printf("tasks\\>");
    for (int i = 0; i < term.width - 11; i++) {
        printf(" ");
    }
    trm_gotoxy(2 + term.prompt_len, y);
}

void trm_redraw_border_if_changed() {
    get_terminal_size(&term.width, &term.height);
    
    if (term.width != term.old_width || term.height != term.old_height) {
        system("cls");
        trm_draw_border(term.width, term.height, term.sep_bar);
        trm_draw_command_prompt();
		trm_clear_screen();
        term.old_width = term.width;
        term.old_height = term.height;
        
        if (cmdline.showing_message) {
            cmdline.showing_message = 0;
        }
        cmd_reset_buffer();
    }
}

// ==================== MESSAGE FUNCTIONS ====================
void cmd_show_message() {
    // Show command executed
	status_set("CMD",cmdline.buffer);
    // Start message timer
    cmdline.showing_message = 1;
    cmdline.message_start_time = GetTickCount();
}

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

void mode_display(void) {
    char *mode_string;
    int mode_width = 5;  // Width of the mode display
    
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
        default:
            mode_string = "?????";
            break;
    }
    
    // Position at right side of command line
    int x = term.width - mode_width - 6;  // 4 for "[ " and " ]"
    int y = term.height - 3;
    
    // Clear the area first
    trm_gotoxy(x, y);
    // Print the mode
    trm_gotoxy(x, y);
    printf("[ %s ]", mode_string);
    fflush(stdout);
}

int cmd_process_commands(ParsedCommand cmd) {
    if (cmd.count == 0) status_clear();
    
    char *command = cmd.tokens[0];
    int result = 0;
    int valid = 1; 
    // Use string comparison in switch
    if ((strcmp(command, "home") == 0)||(strcmp(command, "hm") == 0) ) result = 1;
    else if (strcmp(command, "calendar") == 0) result = 2;
    else if (strcmp(command, "task") == 0) result = 3;
    else if (strcmp(command, "focus") == 0) result = 4;
    else if (strcmp(command, "clear") == 0) result = 5;
    else if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) result = 6;
	else {
		valid = 0;
		char msg[256];
		sprintf(msg, "Unknown command: %s. Type h for help", command);
		status_set("ERR", msg);
	}
    switch (result) {
        case 1:
            trm_gotoxy(2, 2);
			current_mode = HOME;
            printf("Switching to HOME mode");
            break;
        case 2:
            trm_gotoxy(2, 3);
			current_mode = CALENDAR;
            printf("Switching to CALENDAR mode");
            break;
        case 3:
            trm_gotoxy(2, 4);
			current_mode = TASK;
            printf("Switching to TASK mode");
            break;
        case 4:
            trm_gotoxy(2, 5);
			current_mode = FOCUS;
            printf("Switching to FOCUS mode");
            break;
        case 5:
            trm_clear_screen();
            break;
        case 6:
            running = 0;
            break;
        default:
            break;
    }
	return valid;
}


void cmd_clear_message_if_expired() {
    if (cmdline.showing_message && (GetTickCount() - cmdline.message_start_time) >= 1000) {
		status_clear();
        // trm_draw_command_prompt();
        cmdline.showing_message = 0;
        // temporary;
        trm_clear_screen();
    }
}

// ==================== HISTORY FUNCTION MANAGEMENT ==========
void history_load_command() {
    cmd_clear_command_line();
    strcpy(cmdline.buffer, history.history[history.index]);
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
// ==================== INPUT HANDLING ====================


int input_handle_keyboard() {
    if (!_kbhit()) return 1;  // No key pressed, continue running
    
    int ch = _getch();
    
    // Handle extended keys (arrows, F1-F12, etc.)
    if (ch == 224 || ch == 0) {
        ch = _getch();  // Get the extended key code
        switch (ch) {
            case 72:  // Up arrow
                // Handle up arrow 
				status_set("ARW", "^");
                if (history.count > 0 && history.index > 0) {
                    cursor_erase_old();
                    history.index--;
                    history_load_command();
                }
				break;
            case 80:  // Down arrow
                // Handle down arrow 
                status_set("ARW", "_");
                if (history.index < history.count - 1) {
                    cursor_erase_old();
                    history.index++;
                    history_load_command();
                } else {
                    // blank \n 
                    history.index = history.count;
                    cmd_clear_command_line();
                    cmd_reset_buffer();
                }
                break;
            case 75:  // Left arrow
                // Handle left arrow 
                 status_set("ARW", "<");
				 if (!cmdline.showing_message && cmdline.cursor_pos > 0) {
                    cmdline.cursor_pos--;
                    trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
                }
                break;
            case 77:  // Right arrow
                // Handle right arrow - move cursor right
                status_set("ARW", ">");
                if (!cmdline.showing_message && cmdline.cursor_pos < (int)strlen(cmdline.buffer)) {
                    cmdline.cursor_pos++;
                    trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
                }
                break;
        }
        return 1;
    }
    
    // Handle regular keys
    switch (ch) {
		case '\r':  // Enter
			if (strlen(cmdline.buffer) > 0) {
				// Parse command
				ParsedCommandEx cmd;
				cmd.cmd = CMD_NONE;
				cmd.subcmd = 0;
				cmd.arg_count = 0;
				
				// Parse tokens
				ParsedCommand temp = cmd_parse_command_buffer();
				if (temp.count > 0) {
					cmd.cmd = parse_command_string(temp.tokens[0]);
					for (int i = 1; i < temp.count && i < MAX_TOKENS; i++) {
						cmd.args[i-1] = temp.tokens[i];
						cmd.arg_count++;
					}
				}
				
				// Save command for history
				char command_copy[CMD_BUF_SIZE];
				strcpy(command_copy, cmdline.buffer);
				
				// Process command
				int valid = process_command(&cmd, command_copy);
				
				// Add to history
				if (valid) {
					if (history.count >= MAX_HISTORY) {
						for (int i = 1; i < MAX_HISTORY; i++) {
							strcpy(history.history[i-1], history.history[i]);
						}
						history.count = MAX_HISTORY - 1;
					}
					strcpy(history.history[history.count], command_copy);
					history.count++;
					history.index = history.count;
					
					if (cmd.cmd != CMD_CLEAR) {
						status_set("CMD", command_copy);
					}
				}
				
				// Clear command line
				cmd_clear_command_line();
				cmd_reset_buffer();
			} else {
				status_clear();
			}
			break;      
		case '\b':  // Backspace
            if (!cmdline.showing_message) { 
                cmd_remove_char_from_buffer();
			}
            break;
        case 27:  // ESC
            return 0;  // Exit program
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 && !cmdline.showing_message) {
                cmd_add_char_to_buffer(ch);
            }
            break;
    }
    
    return 1;  // Continue running
}

void cursor_position() {
    if (!cmdline.showing_message) {
        trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
    }
}
// ==================== COMMAND PARSING ====================
Command parse_command_string(const char *str) {
    if (strcmp(str, "home") == 0 || strcmp(str, "hm") == 0) return CMD_HOME;
    if (strcmp(str, "help") == 0 || strcmp(str, "h") == 0 || strcmp(str, "?") == 0) return CMD_HELP;
    if (strcmp(str, "clock") == 0 || strcmp(str, "c") == 0) return CMD_CLOCK;
    if (strcmp(str, "calendar") == 0 || strcmp(str, "cal") == 0) return CMD_CALENDAR;
    if (strcmp(str, "task") == 0 || strcmp(str, "tasklist") == 0 || strcmp(str, "t") == 0) return CMD_TASK;
    if (strcmp(str, "focus") == 0 || strcmp(str, "f") == 0) return CMD_FOCUS;
    if (strcmp(str, "exit") == 0 || strcmp(str, "quit") == 0 || strcmp(str, "q") == 0) return CMD_EXIT;
    if (strcmp(str, "history") == 0) return CMD_HISTORY;
    if (strcmp(str, "lock") == 0) return CMD_LOCK;
    if (strcmp(str, "clear") == 0) return CMD_CLEAR;
    return CMD_NONE;
}

// ==================== COMMAND VALIDATION ====================
int validate_command(ParsedCommandEx *cmd) {
    switch (cmd->cmd) {
        case CMD_HOME:
        case CMD_HELP:
        case CMD_CLEAR:
        case CMD_EXIT:
            return cmd->arg_count == 0;  // No args allowed
            
        case CMD_HISTORY:
            return cmd->arg_count <= 1;  // Optional number
            
        case CMD_CLOCK:
            if (cmd->arg_count == 0) return 1;  // Just "clock" is valid
            if (strcmp(cmd->args[0], "-set") != 0) return 0;
            if (cmd->arg_count < 2) return 0;  // Need --time or --date
            
            if (strcmp(cmd->args[1], "--time") == 0) {
                if (cmd->arg_count < 3) return 0;  // Need military/civilian
                if (strcmp(cmd->args[2], "military") != 0 && 
                    strcmp(cmd->args[2], "civilian") != 0) return 0;
                return 1;
            }
            else if (strcmp(cmd->args[1], "--date") == 0) {
                if (cmd->arg_count < 3) return 0;  // Need YMD/MDY/DMY
                if (strcmp(cmd->args[2], "YMD") != 0 && 
                    strcmp(cmd->args[2], "MDY") != 0 && 
                    strcmp(cmd->args[2], "DMY") != 0) return 0;
                return 1;
            }
            return 0;
            
        case CMD_CALENDAR:
            if (cmd->arg_count == 0) return 1;  // Just "calendar" is valid
            // Check flags
            if (strcmp(cmd->args[0], "-month") == 0 || strcmp(cmd->args[0], "-mon") == 0 || strcmp(cmd->args[0], "-m") == 0 ||
                strcmp(cmd->args[0], "-year") == 0 || strcmp(cmd->args[0], "-yr") == 0 || strcmp(cmd->args[0], "-y") == 0 ||
                strcmp(cmd->args[0], "-week") == 0 || strcmp(cmd->args[0], "-wk") == 0 || strcmp(cmd->args[0], "-w") == 0 ||
                strcmp(cmd->args[0], "-day") == 0 || strcmp(cmd->args[0], "-d") == 0) {
                return cmd->arg_count == 2;  // Need flag + value
            }
            return 0;
            
        case CMD_TASK:
            if (cmd->arg_count == 0) return 1;  // Just "task" is valid
            
            // Simple flags
            if (strcmp(cmd->args[0], "-todo") == 0 || strcmp(cmd->args[0], "-td") == 0 ||
                strcmp(cmd->args[0], "-overdue") == 0 || strcmp(cmd->args[0], "-od") == 0 ||
                strcmp(cmd->args[0], "-done") == 0 || strcmp(cmd->args[0], "-d") == 0 ||
                strcmp(cmd->args[0], "-all") == 0) {
                return cmd->arg_count == 1;
            }
            
            // Commands that need arguments
            if (strcmp(cmd->args[0], "-add") == 0 || strcmp(cmd->args[0], "-a") == 0) {
                return cmd->arg_count >= 2;
            }
            if (strcmp(cmd->args[0], "-remove") == 0 || strcmp(cmd->args[0], "-rm") == 0) {
                return cmd->arg_count == 2;
            }
            if (strcmp(cmd->args[0], "-edit") == 0 || strcmp(cmd->args[0], "-e") == 0) {
                return cmd->arg_count >= 2;
            }
            
            // Sort
            if (strcmp(cmd->args[0], "-sort") == 0 || strcmp(cmd->args[0], "-s") == 0) {
                if (cmd->arg_count == 1) return 1;  // Default sort
                if (cmd->arg_count != 2) return 0;
                if (strcmp(cmd->args[1], "default") == 0 || strcmp(cmd->args[1], "creation") == 0 ||
                    strcmp(cmd->args[1], "priority") == 0 || strcmp(cmd->args[1], "deadline") == 0 ||
                    strcmp(cmd->args[1], "status") == 0) return 1;
                return 0;
            }
            
            // Setting
            if (strcmp(cmd->args[0], "-setting") == 0) {
                return cmd->arg_count == 2 && strcmp(cmd->args[1], "done-invisible") == 0;
            }
            return 0;
            
        case CMD_FOCUS:
            if (cmd->arg_count == 0) return 0;  // Needs task ID/name
            if (strcmp(cmd->args[0], "-set-n") == 0) {
                return cmd->arg_count == 2;  // Need task number
            }
            if (strcmp(cmd->args[0], "-set-id") == 0) {
                return cmd->arg_count == 2;  // Need task ID
            }
            return 1;  // Assume it's a task name/id directly
            
        case CMD_LOCK:
            if (cmd->arg_count == 0) return 1;  // Just "lock" is valid
            if (strcmp(cmd->args[0], "-add_usr") == 0 || strcmp(cmd->args[0], "-remove_usr") == 0 || 
                strcmp(cmd->args[0], "-rm_user") == 0) {
                return cmd->arg_count == 2;  // Need username
            }
            return 0;
            
        default:
            return 0;
    }
}

// ==================== PARSED COMMAND EXECUTION ====================
int process_command(ParsedCommandEx *cmd, const char *original_command) {
    if (cmd->cmd == CMD_NONE) {
        char msg[256];
        sprintf(msg, "Unknown command: %s. Type 'help' for help", original_command);
        status_set("ERR", msg);
        return 0;
    }
    
    if (!validate_command(cmd)) {
        status_set("ERR", "Invalid arguments for this command");
        return 0;
    }
    
    switch (cmd->cmd) {
        case CMD_HOME:
            current_mode = HOME;
            trm_clear_screen();
            trm_gotoxy(2, 2);
            printf("=== HOME MODE ===");
            break;
            
        case CMD_HELP:
            show_help();
            break;
            
        case CMD_CLOCK:
            // Handle clock commands
            if (cmd->arg_count == 0) {
                show_current_time();
            } else {
                // Handle -set --time or --date
            }
            break;
            
        case CMD_CALENDAR:
            // Handle calendar commands
            current_mode = CALENDAR;
            trm_clear_screen();
            trm_gotoxy(2, 2);
            printf("=== CALENDAR MODE ===");
            break;
            
        case CMD_TASK:
            // Handle task commands
            current_mode = TASK;
            trm_clear_screen();
            trm_gotoxy(2, 2);
            printf("=== TASK MODE ===");
            break;
            
        case CMD_FOCUS:
            // Handle focus mode
            current_mode = FOCUS;
            trm_clear_screen();
            trm_gotoxy(2, 2);
            printf("=== FOCUS MODE ===");
            break;
            
        case CMD_EXIT:
            running = 0;
            break;
            
        case CMD_HISTORY:
            show_history();
            break;
            
        case CMD_LOCK:
            // Handle lock commands
            break;
            
        case CMD_CLEAR:
            trm_clear_screen();
            break;
            
        default:
            return 0;
    }
    return 1;
}
// ==================== COMMAND FUNCTIONS ====================
void show_help(void) {
    trm_clear_screen();
    trm_gotoxy(2, 2);
    printf("=== HELP ===\n");
    trm_gotoxy(2, 3);
    printf("Commands: home, help, clock, calendar, task, focus, exit, history, lock, clear");
    // Add more help content
}

void show_current_time(void) {
    trm_gotoxy(2, 2);
    printf("Current time: %s", __TIME__);
}

void show_history(void) {
    trm_clear_screen();
    trm_gotoxy(2, 2);
    printf("=== COMMAND HISTORY ===\n");
    int start = (history.count > 10) ? history.count - 10 : 0;
    for (int i = start; i < history.count; i++) {
        trm_gotoxy(2, 3 + (i - start));
        printf("%d: %s", i + 1, history.history[i]);
    }
}
// ==================== CLEANUP FUNCTIONS ====================
void cleanup() {
    trm_restore_original_color();
    cursor_show();
    trm_gotoxy(0, term.height);
    printf("\n");
}

void cleanup_signal_handler(int sig) {
    if (sig == SIGINT) {
        cleanup();
        exit(0);
    }
}

void cleanup_setup() {
    atexit(cleanup);
    signal(SIGINT, cleanup_signal_handler);
}

// ==================== MAIN ====================
int main() 
{
    SetConsoleOutputCP(CP_UTF8);     // ensure it prints utf8 characters
    trm_save_original_color();           // save original consol colors
    trm_set_white_text();                // set current console color to white
    cleanup_setup();                 // setup initialization
    cursor_hide();                   // hide cursor
    while (running) {
        trm_redraw_border_if_changed();  // redraw border if terminal size changed
       // cmd_clear_message_if_expired();  // clear cli message when expired
        cursor_update_blink();		 // cursor blinking
		mode_display();
        if (!input_handle_keyboard()) {
            break;                   // ESC pressed, break
        }
        Sleep(50);                   // sleep for 50 ms
    }
    cleanup();                       // exit cleanup code
    trm_gotoxy(0, term.height);
    printf("\n\nGoodbye!");        // print exit message
    return 0;                        // exit code success
}
