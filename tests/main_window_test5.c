#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

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
    time_t last_blink;
} Cursor;

typedef struct {
    char buffer[256];
    int pos;
    int showing_message;
    time_t message_start_time;
} CommandLine;

typedef struct {
    char history[256][256];
    int count;
    int index;
} History;

// --- command struct
typedef struct {
    char *tokens[64];
    int count;
} ParsedCommand;

// ==================== GLOBAL VARIABLES ====================
Terminal term = {0, 0, -1, -1, 4, 8};
int running = 1;
Cursor cursor = {0, 0, 1, 0, 0};
CommandLine cmdline = {{0}, 0, 0, 0};
History history = {{{0}}, 0, 0};

// ==================== TERMINAL FUNCTIONS ====================
void get_terminal_size(int *width, int *height) {
    getmaxyx(stdscr, *height, *width);
}

void gotoxy(int x, int y) {
    move(y, x);
}

// ==================== CURSOR FUNCTIONS ====================
void show_cursor() {
    curs_set(1);  // Show cursor
}

void hide_cursor() {
    curs_set(0);  // Hide cursor
}

void erase_old_cursor() {
    int x = 2 + term.prompt_len + cursor.prev_pos;
    int y = term.height - 3;
    move(y, x);
    // restore underlying character
    if (cursor.prev_pos < (int)strlen(cmdline.buffer)) {
        addch(cmdline.buffer[cursor.prev_pos]);
    } else {
        addch(' ');
    }
}

void draw_manual_cursor() {
    int x = 2 + term.prompt_len + cmdline.pos;
    int y = term.height - 3;
    move(y, x);
    addch('_');
    cursor.prev_pos = cmdline.pos;
    refresh();
}

void update_cursor_blink() {
    time_t now = time(NULL);
    if (now - cursor.last_blink >= 1) {  // Blink every second
        cursor.last_blink = now;
        if (cursor.visible) {
            erase_old_cursor();
            cursor.visible = 0;
        } else {
            draw_manual_cursor();
            cursor.visible = 1;
        }
        refresh();
    }
}

// ==================== BUFFER FUNCTIONS ====================
void reset_buffer() {
    cmdline.buffer[0] = '\0';
    cmdline.pos = 0;
}

void clear_command_line() {
    move(term.height - 3, 2 + term.prompt_len);
    for (int i = 0; i < (term.width - 11); i++) {
        addch(' ');
    }
    move(term.height - 3, 2 + term.prompt_len);
    refresh();
}

void add_char_to_buffer(char ch) {
    if (cmdline.pos < 255) {
        // If inserting in the middle, shift characters right
        if (cmdline.pos < (int)strlen(cmdline.buffer)) {
            for (int i = strlen(cmdline.buffer); i >= cmdline.pos; i--) {
                cmdline.buffer[i + 1] = cmdline.buffer[i];
            }
        }
        
        cmdline.buffer[cmdline.pos] = ch;
        cmdline.pos++;
        
        int max_display = term.width - 2 - (2 + term.prompt_len);
        int len = strlen(cmdline.buffer);
        
        if (len <= max_display) {
            move(term.height - 3, 2 + term.prompt_len + cmdline.pos - 2);
            addch(ch);
        } else {
            int start = len - max_display;
            if (cmdline.pos > len) {
                start = len - max_display;
            }
            if (cmdline.pos - start >= max_display) {
                start = cmdline.pos - max_display + 1;
            }
            if (start < 0) start = 0;
            
            move(term.height - 3, 2 + term.prompt_len);
            
            int visible_len = (len - start < max_display) ? len - start : max_display;
            for (int i = 0; i < visible_len; i++) {
                addch(cmdline.buffer[start + i]);
            }
            
            for (int i = visible_len; i < max_display; i++) {
                addch(' ');
            }
            
            int cursor_offset = cmdline.pos - start;
            move(term.height - 3, 2 + term.prompt_len + cursor_offset);
        }
        refresh();
    } else {
        move(term.height - 2, 2);
        printw("[ ERR ]: command line buffer full (max 255)");
        move(term.height - 3, term.width - 2);
        refresh();
    }
}

void remove_char_from_buffer() {
    if (cmdline.pos > 0) {
        for (int i = cmdline.pos - 1; i < (int)strlen(cmdline.buffer); i++) {
            cmdline.buffer[i] = cmdline.buffer[i + 1];
        }
        cmdline.pos--;
        
        int max_display = term.width - 1 - (2 + term.prompt_len);
        int len = strlen(cmdline.buffer);
        
        if (len <= max_display) {
            move(term.height - 3, 2 + term.prompt_len + cmdline.pos);
            addch(' ');
            move(term.height - 3, 2 + term.prompt_len + cmdline.pos);
        } else {
            int start = len - max_display;
            if (cmdline.pos < start) {
                start = cmdline.pos;
            }
            if (start < 0) start = 0;
            
            move(term.height - 3, 2 + term.prompt_len);
            
            int visible_len = (len - start < max_display) ? len - start : max_display;
            for (int i = 0; i < visible_len; i++) {
                addch(cmdline.buffer[start + i]);
            }
            
            for (int i = visible_len; i < max_display; i++) {
                addch(' ');
            }
            
            int cursor_offset = cmdline.pos - start;
            move(term.height - 3, 2 + term.prompt_len + cursor_offset);
        }
        refresh();
    }
}

// ==================== DRAWING FUNCTIONS ====================
void clear_screen() {
    for (int x = 1; x < term.width - 1; x++) {
        for (int y = 1; y < term.height - term.sep_bar; y++) {
            move(y, x);
            addch(' ');
        }
    }
    refresh();
}

void draw_border(int width, int height, int sep_bar) {
    // Top border
    move(0, 0);
    addch(ACS_ULCORNER);
    for (int x = 1; x < width - 1; x++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    
    // Bottom border
    move(height - 1, 0);
    addch(ACS_LLCORNER);
    for (int x = 1; x < width - 1; x++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    
    // Separator bar
    move(height - sep_bar, 0);
    addch(ACS_LTEE);
    for (int x = 1; x < width - 1; x++) {
        addch(ACS_HLINE);
    }
    addch(ACS_RTEE);
    
    // Left and right borders
    for (int y = 1; y < height - 1; y++) {
        if (y == height - sep_bar) continue;
        
        move(y, 0);
        addch(ACS_VLINE);
        
        move(y, width - 1);
        addch(ACS_VLINE);
    }
    refresh();
}

void draw_command_prompt() {
    int y = term.height - 3;
    move(y, 2);
    printw("tasks\\>");
    for (int i = 0; i < term.width - 11; i++) {
        addch(' ');
    }
    move(y, 2 + term.prompt_len);
    refresh();
}

void redraw_border_if_changed() {
    get_terminal_size(&term.width, &term.height);
    
    if (term.width != term.old_width || term.height != term.old_height) {
        clear();
        draw_border(term.width, term.height, term.sep_bar);
        draw_command_prompt();
        clear_screen();
        term.old_width = term.width;
        term.old_height = term.height;
        
        if (cmdline.showing_message) {
            cmdline.showing_message = 0;
        }
        reset_buffer();
        refresh();
    }
}

// ==================== MESSAGE FUNCTIONS ====================
void show_command_message() {
    clear_command_line();
    
    move(term.height - 2, 2);
    printw("[ CMD ]: %s", cmdline.buffer);
    
    reset_buffer();
    
    cmdline.showing_message = 1;
    cmdline.message_start_time = time(NULL);
    refresh();
}

ParsedCommand parse_command_buffer() {
    ParsedCommand cmd;
    cmd.count = 0;
    static char buffer_copy[256];
    strcpy(buffer_copy, cmdline.buffer);
    char *token = strtok(buffer_copy, " ");

    while (token != NULL && cmd.count < 64) {
        cmd.tokens[cmd.count] = token;
        cmd.count++;
        token = strtok(NULL, " ");
    }
    return cmd;
}

void process_commands(ParsedCommand cmd) {
    for (int i = 0; i < cmd.count; i++) {
        move(5 + i, 5);
        printw("%s", cmd.tokens[i]);
    }
    refresh();
}

void clear_message_if_expired() {
    if (cmdline.showing_message && (time(NULL) - cmdline.message_start_time) >= 1) {
        move(term.height - 2, 2);
        for (int i = 0; i < term.width - 10; i++) {
            addch(' ');
        }
        draw_command_prompt();
        cmdline.showing_message = 0;
        clear_screen();
        refresh();
    }
}

// ==================== HISTORY FUNCTION MANAGEMENT ==========
void load_history_command() {
    clear_command_line();
    strcpy(cmdline.buffer, history.history[history.index]);
    cmdline.pos = strlen(cmdline.buffer);
    move(term.height - 3, 2 + term.prompt_len);
    printw("%s", cmdline.buffer);
    draw_manual_cursor();
    refresh();
}

// ==================== INPUT HANDLING ====================
int handle_keyboard_input() {
    int ch = getch();
    if (ch == ERR) return 1;
    
    switch (ch) {
        case KEY_UP:  // Up arrow
            if (history.count > 0 && history.index > 0) {
                erase_old_cursor();
                history.index--;
                load_history_command();
            }
            move(term.height - 2, 2);
            printw("[ ARW ]: ^");
            refresh();
            break;
            
        case KEY_DOWN:  // Down arrow
            move(term.height - 2, 2);
            printw("[ ARW ]: _");
            if (history.index < history.count - 1) {
                erase_old_cursor();
                history.index++;
                load_history_command();
            } else {
                history.index = history.count;
                clear_command_line();
                reset_buffer();
            }
            refresh();
            break;
            
        case KEY_LEFT:  // Left arrow
            move(term.height - 2, 2);
            printw("[ ARW ]: <");
            if (!cmdline.showing_message && cmdline.pos > 0) {
                cmdline.pos--;
                move(term.height - 3, 2 + term.prompt_len + cmdline.pos);
            }
            refresh();
            break;
            
        case KEY_RIGHT:  // Right arrow
            move(term.height - 2, 2);
            printw("[ ARW ]: >");
            if (!cmdline.showing_message && cmdline.pos < (int)strlen(cmdline.buffer)) {
                cmdline.pos++;
                move(term.height - 3, 2 + term.prompt_len + cmdline.pos);
            }
            refresh();
            break;
            
        case '\n':  // Enter
        case '\r':
            if (strlen(cmdline.buffer) > 0) {
                ParsedCommand cmd = parse_command_buffer();
                process_commands(cmd);
                
                if (history.count >= 256) {
                    for (int i = 1; i < 256; i++) {
                        strcpy(history.history[i-1], history.history[i]);
                    }
                    history.count = 255;
                }
                
                strcpy(history.history[history.count], cmdline.buffer);
                history.count++;
                history.index = history.count;
                show_command_message();
            }
            break;
            
        case KEY_BACKSPACE:
        case 127:  // Delete/Backspace
            if (!cmdline.showing_message) {
                remove_char_from_buffer();
            }
            break;
            
        case 27:  // ESC
            return 0;
            
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 && !cmdline.showing_message) {
                add_char_to_buffer(ch);
            }
            break;
    }
    
    return 1;
}

void position_cursor() {
    if (!cmdline.showing_message) {
        move(term.height - 3, 2 + term.prompt_len + cmdline.pos);
        refresh();
    }
}

// ==================== CLEANUP FUNCTIONS ====================
void cleanup() {
    endwin();
    printf("\n\nGoodbye!\n");
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
int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor initially
    
    setup_cleanup();
    
    // Get initial terminal size
    get_terminal_size(&term.width, &term.height);
    term.old_width = term.width;
    term.old_height = term.height;
    
    // Draw initial UI
    draw_border(term.width, term.height, term.sep_bar);
    draw_command_prompt();
    refresh();
    
    while (running) {
        redraw_border_if_changed();
        clear_message_if_expired();
        update_cursor_blink();
        if (!handle_keyboard_input()) {
            break;
        }
        napms(50);  // Sleep for 50ms
    }
    
    cleanup();
    return 0;
}
