#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

// ==================== GLOBAL VARIABLES ====================
int width = 0;
int height = 0;
int old_width = -1;
int old_height = -1;
int sep_bar = 4;
int running = 1;

char command_buffer[256] = "";
int cursor_pos = 0;
int showing_message = 0;
DWORD message_start_time = 0;

int cursor_visible = 1;
DWORD last_cursor_blink = 0;

// command struct
typedef struct {
    char *tokens[64];
    int count;
} ParsedCommand;

//===================== TERMINAL COLORS =====================
WORD original_color = 0;

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
    SetConsoleTextAttribute(hConsole, 7);
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

void draw_manual_cursor() {
    int x = 2 + 8 + cursor_pos;
    int y = height - 3;
    
    gotoxy(x, y);
    if (cursor_visible) {
        printf("|");
    } else {
        printf(" ");
    }
    gotoxy(x, y); // Return cursor to the same position
}

void update_cursor_blink() {
    DWORD now = GetTickCount();
    if (now - last_cursor_blink >= 500) {
        cursor_visible = !cursor_visible;
        last_cursor_blink = now;
        draw_manual_cursor();
    }
}

// ==================== BUFFER FUNCTIONS ====================
void reset_buffer() {
    command_buffer[0] = '\0';
    cursor_pos = 0;
}

void redraw_command_buffer() {
    int y = height - 3;
    gotoxy(2 + 8, y);
    // Print the entire buffer
    printf("%-*s", (int)strlen(command_buffer), command_buffer);
    // Clear any remaining characters
    for (int i = strlen(command_buffer); i < 255; i++) {
        printf(" ");
    }
    gotoxy(2 + 8 + cursor_pos, y);
}

void add_char_to_buffer(char ch) {
    if (cursor_pos < 255) {
        // Shift characters to the right if inserting in the middle
        if (cursor_pos < (int)strlen(command_buffer)) {
            for (int i = strlen(command_buffer); i >= cursor_pos; i--) {
                command_buffer[i + 1] = command_buffer[i];
            }
        }
        command_buffer[cursor_pos] = ch;
        cursor_pos++;
        redraw_command_buffer();
    }
}

void remove_char_from_buffer() {
    if (cursor_pos > 0) {
        // Shift characters left
        for (int i = cursor_pos - 1; i < (int)strlen(command_buffer); i++) {
            command_buffer[i] = command_buffer[i + 1];
        }
        cursor_pos--;
        redraw_command_buffer();
    }
}

// ==================== DRAWING FUNCTIONS ====================
void draw_border(int width, int height, int sep_bar) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            gotoxy(x, y);
            if (x == 0 && y == 0) printf("╔");
            else if (x == width - 1 && y == 0) printf("╗");
            else if (x == 0 && y == height - 1) printf("╚");
            else if (x == width - 1 && y == height - 1) printf("╝");
            else if (x == 0 && y == height - sep_bar) printf("╠");
            else if (x == width - 1 && y == height - sep_bar) printf("╣");
            else if (y == height - sep_bar) printf("═");
            else if (y == 0 || y == height - 1) printf("═");
            else if (x == 0 || x == width - 1) printf("║");
            else printf(" ");
        }
    }
}

void draw_command_prompt() {
    int y = height - 3;
    gotoxy(2, y);
    printf("tasks\\>");
    // Clear the rest of the command line
    for (int i = 0; i < width - 11; i++) {
        printf(" ");
    }
    gotoxy(2 + 8, y);
}

void redraw_border_if_changed() {
    get_terminal_size(&width, &height);
    if (width != old_width || height != old_height) {
        system("cls");
        draw_border(width, height, sep_bar);
        draw_command_prompt();
        old_width = width;
        old_height = height;
        
        if (showing_message) {
            showing_message = 0;
        }
        redraw_command_buffer();
    }
}

// ==================== MESSAGE FUNCTIONS ====================
void show_command_message() {
    showing_message = 1;
    message_start_time = GetTickCount();
    
    // Show command executed
    gotoxy(2, height - 2);
    printf("[ CMD ]: %s", command_buffer);
    // Clear the rest of the line
    for (int i = 8 + strlen(command_buffer); i < width - 2; i++) {
        printf(" ");
    }
}

ParsedCommand parse_command_buffer() {
    ParsedCommand cmd;
    cmd.count = 0;
    static char buffer_copy[256];
    strcpy(buffer_copy, command_buffer);
    char *token = strtok(buffer_copy, " ");
    
    while (token != NULL && cmd.count < 64) {
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

void clear_screen() {
    for (int x = 1; x < width - 1; x++) {
        for (int y = 1; y < height - 6; y++) {
            gotoxy(x, y);
            printf(" ");
        }
    }
}

void clear_message_if_expired() {
    if (showing_message && (GetTickCount() - message_start_time) >= 1000) {
        gotoxy(2, height - 2);
        for (int i = 0; i < width - 10; i++) {
            printf(" ");
        }
        draw_command_prompt();
        redraw_command_buffer();
        showing_message = 0;
        clear_screen();
    }
}

// ==================== INPUT HANDLING ====================
int handle_keyboard_input() {
    if (!_kbhit()) return 1;
    
    int ch = _getch();
    
    // Handle extended keys
    if (ch == 224 || ch == 0) {
        ch = _getch();
        switch (ch) {
            case 72:  // Up arrow
                gotoxy(2, height - 2); 
                printf("[ ARW ]: ^");
                break;
            case 80:  // Down arrow
                gotoxy(2, height - 2); 
                printf("[ ARW ]: _");
                break;
            case 75:  // Left arrow
                if (!showing_message && cursor_pos > 0) {
                    cursor_pos--;
                    gotoxy(2, height - 2); 
                    printf("[ ARW ]: <");
                    redraw_command_buffer();
                }
                break;
            case 77:  // Right arrow
                if (!showing_message && cursor_pos < (int)strlen(command_buffer)) {
                    cursor_pos++;
                    gotoxy(2, height - 2); 
                    printf("[ ARW ]: >");
                    redraw_command_buffer();
                }
                break;
        }
        return 1;
    }
    
    // Handle regular keys
    switch (ch) {
        case '\r':  // Enter
            if (strlen(command_buffer) > 0) {
                ParsedCommand cmd = parse_command_buffer();
                process_commands(cmd);
                show_command_message();
                reset_buffer();
                redraw_command_buffer();
            }
            break;
        case '\b':  // Backspace
            if (!showing_message) {
                remove_char_from_buffer();
            }
            break;
        case 27:  // ESC
            return 0;
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 && !showing_message) {
                add_char_to_buffer(ch);
            }
            break;
    }
    
    return 1;
}

// ==================== CLEANUP FUNCTIONS ====================
void cleanup() {
    restore_original_color();
    show_cursor();
    gotoxy(0, height);
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
int main() {
    SetConsoleOutputCP(CP_UTF8);
    save_original_color();
    set_white_text();
    setup_cleanup();
    hide_cursor();
    
    while (running) {
        redraw_border_if_changed();
        clear_message_if_expired();
        update_cursor_blink();
        if (!handle_keyboard_input()) {
            break;
        }
        Sleep(50);
    }
    
    cleanup();
    gotoxy(0, height);
    printf("\n\nGoodbye!\n");
    return 0;
}
