#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

// ==================== GLOBAL VARIABLES ====================
// --- terminal width
int width = 0;
int height = 0;
int old_width = -1;
int old_height = -1;
int sep_bar = 4;
// --- state
int running = 1;
// --- command line and cursor
int prev_cursor_pos = 0;
char command_buffer[256] = "";
int cursor_pos = 0;
int showing_message = 0;
DWORD message_start_time = 0;
int cursor_visible = 1;
DWORD last_cursor_blink = 0;
// --- command history
char command_history[256][256];
int  history_count = 0;
int  history_index = 0;

// --- command struct
typedef struct {
	char *tokens[64];
	int count;
} ParsedCommand;

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
    int x = 2 + 8 + prev_cursor_pos;
    int y = height - 3;
    gotoxy(x, y);
    // restore underlying character
    if (prev_cursor_pos < strlen(command_buffer)) {
        printf("%c", command_buffer[prev_cursor_pos]);
    }
    else {
        printf(" ");
    }
}

void draw_manual_cursor() {
    int x = 2 + 8 + cursor_pos;
    int y = height - 3;
    gotoxy(x, y);

    printf("_");

    prev_cursor_pos = cursor_pos;

    fflush(stdout);
}

void update_cursor_blink() {
    DWORD now = GetTickCount();
    if (now - last_cursor_blink >= 500) {
        last_cursor_blink = now;
        if (cursor_visible) {
            // hide cursor
            erase_old_cursor();
            cursor_visible = 0;
        }
        else {
            // show cursor
            draw_manual_cursor();
            cursor_visible = 1;
        }
    }
}


// ==================== BUFFER FUNCTIONS ====================
void reset_buffer() {
    command_buffer[0] = '\0';
    cursor_pos = 0;
}

void clear_command_line() {
    gotoxy(2 + 8, height - 3);
    for (int i = 0; i < cursor_pos; i++) {
        printf(" ");
    }
    gotoxy(2 + 8, height - 3);
}

void add_char_to_buffer(char ch) {
    if (cursor_pos < 255) {
        command_buffer[cursor_pos] = ch;
        gotoxy(2 + 8 + cursor_pos, height - 3);
        printf("%c", ch);
        cursor_pos++;
        command_buffer[cursor_pos] = '\0';
    }
}

void remove_char_from_buffer() {
    if (cursor_pos > 0) {
        cursor_pos--;
        command_buffer[cursor_pos] = '\0';
        gotoxy(2 + 8 + cursor_pos, height - 3);
        printf(" ");
        gotoxy(2 + 8 + cursor_pos, height - 3);
    }
}

// ==================== DRAWING FUNCTIONS ====================
void draw_border(int width, int height, int sep_bar) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
			gotoxy(x,y);
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
		reset_buffer();
    }
}

// ==================== MESSAGE FUNCTIONS ====================
void show_command_message() {
    // Clear command line
    clear_command_line();
    
    // Show command executed
    gotoxy(2, height - 2);
    printf("[ CMD ]: %s", command_buffer);
    
    // Reset buffer
    reset_buffer();
    
    // Start message timer
    showing_message = 1;
    message_start_time = GetTickCount();
}

ParsedCommand parse_command_buffer(){
	ParsedCommand cmd;
	
	cmd.count = 0;
	static char buffer_copy[256];
	strcpy( buffer_copy, command_buffer);
	char *token = strtok(buffer_copy, " ");

	while (token != NULL && cmd.count < 64) {
		cmd.tokens[cmd.count] = token;
		cmd.count++;
		token = strtok(NULL, " " );
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
		for (int y = 1; y < height - 6; y ++){
			gotoxy(x,y);
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
        showing_message = 0;
		// temporary;
		clear_screen();
	}
}

// ==================== HISTORY FUNCTION MANAGEMENT ==========
void load_history_command() {
	clear_command_line();
	strcpy(command_buffer, command_history[history_index]);
	cursor_pos = strlen(command_buffer);
	gotoxy(2 + 8, height - 3);
	printf("%s", command_buffer);
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
				if (history_count > 0 && history_index > 0) {
					erase_old_cursor();
					history_index--;
					load_history_command();
				}
                gotoxy(2,height - 2); printf("[ ARW ]: ^");
				break;
            case 80:  // Down arrow
                // Handle down arrow 
                gotoxy(2,height - 2); printf("[ ARW ]: _");
				if (history_index < history_count - 1) {
					erase_old_cursor();
					history_index++;
					load_history_command();
				} else {
					// blank \n 
					history_index = history_count;
					clear_command_line();
					reset_buffer();
				}
                break;
            case 75:  // Left arrow
                // Handle left arrow 
                gotoxy(2,height - 2); printf("[ ARW ]: <");
                if (!showing_message && cursor_pos > 0) {
                    cursor_pos--;
                    gotoxy(2 + 8 + cursor_pos, height - 3);
                }
                break;
            case 77:  // Right arrow
                // Handle right arrow - move cursor right
                gotoxy(2,height - 2); printf("[ ARW ]: >");
                if (!showing_message && cursor_pos < (int)strlen(command_buffer)) {
                    cursor_pos++;
                    gotoxy(2 + 8 + cursor_pos, height - 3);
                }
                break;
        }
        return 1;
    }
    
    // Handle regular keys
    switch (ch) {
        case '\r':  // Enter
            if (strlen(command_buffer) > 0) {  // Only show if there's a command
				ParsedCommand cmd = parse_command_buffer();
				process_commands(cmd);
				
				// history count

				if (history_count >= 256) {
					for (int i = 1; i < 256; i++) {
						strcpy(command_history[i-1],command_history[i]);
					}
					history_count = 255;
				}

				strcpy(command_history[history_count], command_buffer);
				history_count++;
				history_index = history_count;
                show_command_message();
            }
            break;
        case '\b':  // Backspace
            if (!showing_message) { 
				remove_char_from_buffer();
				// draw_manual_cursor();
			}
            break;
        case 27:  // ESC
            return 0;  // Exit program
        default:  // Printable characters
            if (ch >= 32 && ch <= 126 && !showing_message) {
                add_char_to_buffer(ch);
				// draw_manual_cursor();
            }
            break;
    }
    
    return 1;  // Continue running
}

void position_cursor() {
    if (!showing_message) {
        gotoxy(2 + 8 + cursor_pos, height - 3);
    }
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
    SetConsoleOutputCP(CP_UTF8);	 // ensure it prints utf8 characters
	save_original_color();			 // save original consol colors
	set_white_text();				 // set current console color to white
    setup_cleanup();				 // setup initialization
    hide_cursor();					 // hide cursor
    while (running) {
        redraw_border_if_changed();  // redraw border if terminal size changed
        clear_message_if_expired();	 // clear cli message when expired
        // position_cursor();           // position the cursor to the desired state
		update_cursor_blink();
        if (!handle_keyboard_input()) {
            break;					 // ESC pressed, break
        }
        Sleep(50);					 // sleep for 60 ms
    }
    cleanup();					     // exit cleanup code
    gotoxy(0, height);
    printf("\n\nGoodbye!\n");		 // print exit message
    return 0;						 // exit code success
}
