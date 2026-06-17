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

//===================== TERMINAL COLORS =====================

WORD original_color = 0;  // Store original color globally

void save_original_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    original_color = csbi.wAttributes;
}

void set_console_color(int foreground, int background) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (background * 16) + foreground);
}

void restore_original_color() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, original_color);
}


void set_white_text() {
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

void gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ===================== STATUS LINE ========================
void clear_status(void) {
    gotoxy(2, term.height - 2);
    for (int i = 0; i < term.width - 4; i++)
        putchar(' ');
    fflush(stdout);
}

void draw_status(const char *msg) {
    gotoxy(2, term.height - 2);
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

void set_status(const char *tag, const char *msg) {
    char buf[512];
    snprintf(buf, sizeof(buf), "[ %s ]: %s", tag, msg);
    draw_status(buf);
}
// ==================== CURSOR FUNCTIONS ====================
void show_cursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void hide_cursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void erase_old_cursor() {
    int screen_pos =
        cursor.prev_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;

    gotoxy(x, y);

    if (cursor.prev_pos < strlen(cmdline.buffer))
        putchar(cmdline.buffer[cursor.prev_pos]);
    else
        putchar(' ');
}

void draw_manual_cursor() {
    int screen_pos =
        cmdline.cursor_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int visible_width = term.width - 4 - term.prompt_len;

    if (screen_pos >= visible_width) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;
    gotoxy(x, y);
    putchar('_');
    cursor.prev_pos = cmdline.cursor_pos;
    fflush(stdout);
}

void update_cursor_blink() {
    DWORD now = GetTickCount();
    if (now - cursor.last_blink >= 500) {
        cursor.last_blink = now;
        if (cursor.visible) {
            // hide cursor
            erase_old_cursor();
            cursor.visible = 0;
        }
        else {
            // show cursor
            draw_manual_cursor();
            cursor.visible = 1;
        }
    }
}


// ==================== BUFFER FUNCTIONS ====================
void reset_buffer() {
    memset(cmdline.buffer, 0, sizeof(cmdline.buffer));
    cmdline.cursor_pos = 0;
	cmdline.scroll_offset = 0;
}

void clear_command_line() {
    gotoxy(2 + term.prompt_len, term.height - 3);
    for (int i = 0; i < (term.width - 11); i++) {
        printf(" ");
    }
    gotoxy(2 + term.prompt_len, term.height - 3);
}
  

void redraw_buffer(void) {
    int y = term.height - 3;
    int visible_width =
        term.width - 5 - term.prompt_len;
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
    gotoxy(2 + term.prompt_len, y);

    for (int i = 0; i < visible_width; i++)
    {
        int idx = cmdline.scroll_offset + i;
        if (idx < len) {
            putchar(cmdline.buffer[idx]);
        } else {
            putchar(' '); 
        }
    }

    gotoxy(
        2 + term.prompt_len +
        (cmdline.cursor_pos -
         cmdline.scroll_offset),
        y
    );

    fflush(stdout);
}

void add_char_to_buffer(char ch) {
    int len = strlen(cmdline.buffer);
	if (len >= CMD_BUF_SIZE - 1){
		set_status("ERR","command line buffer maximum reached");
        return;
	}
    for (int i = len; i >= cmdline.cursor_pos; i--)
    {
        cmdline.buffer[i + 1] =
            cmdline.buffer[i];
    }

    cmdline.buffer[cmdline.cursor_pos] = ch;
    cmdline.cursor_pos++;

    redraw_buffer();
}

void remove_char_from_buffer(void) {
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
	if (len < CMD_BUF_SIZE - 1) clear_status();
    redraw_buffer();
}

// ==================== DRAWING FUNCTIONS ====================
void clear_screen() {
    for (int x = 1; x < term.width - 1; x++) {
        for (int y = 1; y < term.height - term.sep_bar; y ++){
            gotoxy(x,y);
            printf(" ");
        }
    }
}

void draw_border(int width, int height, int sep_bar) {
    // Top border (y = 0)
    gotoxy(0, 0);
    printf("╔");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╗");
    
    // Bottom border (y = height - 1)
    gotoxy(0, height - 1);
    printf("╚");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╝");
    
    // Separator bar (y = height - sep_bar)
    gotoxy(0, height - sep_bar);
    printf("╠");
    for (int x = 1; x < width - 1; x++) {
        printf("═");
    }
    printf("╣");
    
    // Left and right borders
    for (int y = 1; y < height - 1; y++) {
        if (y == height - sep_bar) continue;  // Already drawn
        
        gotoxy(0, y);
        printf("║");
        
        gotoxy(width - 1, y);
        printf("║");
    }
}

void draw_command_prompt() {
    int y = term.height - 3;
    gotoxy(2, y);
    printf("tasks\\>");
    for (int i = 0; i < term.width - 11; i++) {
        printf(" ");
    }
    gotoxy(2 + term.prompt_len, y);
}

void redraw_border_if_changed() {
    get_terminal_size(&term.width, &term.height);
    
    if (term.width != term.old_width || term.height != term.old_height) {
        system("cls");
        draw_border(term.width, term.height, term.sep_bar);
        draw_command_prompt();
		clear_screen();
        term.old_width = term.width;
        term.old_height = term.height;
        
        if (cmdline.showing_message) {
            cmdline.showing_message = 0;
        }
        reset_buffer();
    }
}

// ==================== MESSAGE FUNCTIONS ====================
void show_command_message() {
    // Clear command line
    clear_command_line();
    
    // Show command executed
	set_status("CMD",cmdline.buffer);
    // Reset buffer
    reset_buffer();
    
    // Start message timer
    cmdline.showing_message = 1;
    cmdline.message_start_time = GetTickCount();
}

ParsedCommand parse_command_buffer(){
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

void process_commands(ParsedCommand cmd) {
    // sample text. we will use a switch statement to do shit later
    for (int i = 0; i< cmd.count; i++) {
        gotoxy(5,5 + i);
        printf("%s",cmd.tokens[i]);
    }
}


void clear_message_if_expired() {
    if (cmdline.showing_message && (GetTickCount() - cmdline.message_start_time) >= 1000) {
		clear_status();
        draw_command_prompt();
        cmdline.showing_message = 0;
        // temporary;
        clear_screen();
    }
}

// ==================== HISTORY FUNCTION MANAGEMENT ==========
void load_history_command() {
    clear_command_line();
    strcpy(cmdline.buffer, history.history[history.index]);
    cmdline.cursor_pos = strlen(cmdline.buffer);
    gotoxy(2 + term.prompt_len, term.height - 3);
    printf("%s", cmdline.buffer);
    draw_manual_cursor();
}
// ==================== INPUT HANDLING ====================


int handle_keyboard_input() {
    if (!_kbhit()) return 1;  // No key pressed, continue running
    
    int ch = _getch();
    
    // Handle extended keys (arrows, F1-F12, etc.)
    if (ch == 224 || ch == 0) {
        ch = _getch();  // Get the extended key code
        switch (ch) {
            case 72:  // Up arrow
                // Handle up arrow 
				set_status("ARW", "^");
                if (history.count > 0 && history.index > 0) {
                    erase_old_cursor();
                    history.index--;
                    load_history_command();
                }
				break;
            case 80:  // Down arrow
                // Handle down arrow 
                set_status("ARW", "_");
                if (history.index < history.count - 1) {
                    erase_old_cursor();
                    history.index++;
                    load_history_command();
                } else {
                    // blank \n 
                    history.index = history.count;
                    clear_command_line();
                    reset_buffer();
                }
                break;
            case 75:  // Left arrow
                // Handle left arrow 
                 set_status("ARW", "<");
				 if (!cmdline.showing_message && cmdline.cursor_pos > 0) {
                    cmdline.cursor_pos--;
                    gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
                }
                break;
            case 77:  // Right arrow
                // Handle right arrow - move cursor right
                set_status("ARW", ">");
                if (!cmdline.showing_message && cmdline.cursor_pos < (int)strlen(cmdline.buffer)) {
                    cmdline.cursor_pos++;
                    gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
                }
                break;
        }
        return 1;
    }
    
    // Handle regular keys
    switch (ch) {
        case '\r':  // Enter
            if (strlen(cmdline.buffer) > 0) {  // Only show if there's a command
                ParsedCommand cmd = parse_command_buffer();
                process_commands(cmd);
                
                // history count
                if (history.count >= MAX_HISTORY) {
                    for (int i = 1; i < MAX_HISTORY; i++) {
                        strcpy(history.history[i-1], history.history[i]);
                    }
                    history.count = MAX_HISTORY - 1;
                }

                strcpy(history.history[history.count], cmdline.buffer);
                history.count++;
                history.index = history.count;
                show_command_message();
            }
            break;
        case '\b':  // Backspace
            if (!cmdline.showing_message) { 
                remove_char_from_buffer();
			}
            break;
        case 27:  // ESC
            return 0;  // Exit program
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 && !cmdline.showing_message) {
                add_char_to_buffer(ch);
            }
            break;
    }
    
    return 1;  // Continue running
}

void position_cursor() {
    if (!cmdline.showing_message) {
        gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
    }
}
// ==================== CLEANUP FUNCTIONS ====================
void cleanup() {
    restore_original_color();
    show_cursor();
    gotoxy(0, term.height);
    printf("\n");
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        cleanup();
        exit(0);
    }
}

void setup_cleanup() {
    atexit(cleanup);
    signal(SIGINT, signal_handler);
}

// ==================== MAIN ====================
int main() 
{
    SetConsoleOutputCP(CP_UTF8);     // ensure it prints utf8 characters
    save_original_color();           // save original consol colors
    set_white_text();                // set current console color to white
    setup_cleanup();                 // setup initialization
    hide_cursor();                   // hide cursor
    while (running) {
        redraw_border_if_changed();  // redraw border if terminal size changed
        clear_message_if_expired();  // clear cli message when expired
        update_cursor_blink();		 // cursor blinking
        if (!handle_keyboard_input()) {
            break;                   // ESC pressed, break
        }
        Sleep(50);                   // sleep for 50 ms
    }
    cleanup();                       // exit cleanup code
    gotoxy(0, term.height);
    printf("\n\nGoodbye!\n");        // print exit message
    return 0;                        // exit code success
}
