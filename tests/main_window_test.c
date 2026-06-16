#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

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

void draw_border(int width, int height, int sep_bar) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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


void draw_command_prompt(int width, int height) {
    int y = height - 3;
    gotoxy(2, y);
    printf("tasks\\>");
    // Clear the rest of the line to prevent ghost characters
    for (int i = 0; i < width - 10; i++) {
        printf(" ");
    }
    gotoxy(2 + 8, y);  // Reset cursor to input position
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    hide_cursor();
    
    int running = 1;
    int width = 0, height = 0;
    int old_width = -1, old_height = -1;
    int sep_bar = 4;
    
    char command_buffer[256] = "";
    int cursor_pos = 0;
    int ch;
    int prompt_y, prompt_x;
    
    // Flag to track if we're showing a message
    int showing_message = 0;
    DWORD message_start_time = 0;
    
    while (running) {
        get_terminal_size(&width, &height);
        
        // Only redraw border when terminal size changes
        if (width != old_width || height != old_height) {
            system("cls");
            draw_border(width, height, sep_bar);
            draw_command_prompt(width, height);
            old_width = width;
            old_height = height;
            
            // If we were showing a message, clear it and reprompt
            if (showing_message) {
                showing_message = 0;
            }
			command_buffer[0] = '\0';
			cursor_pos = 0;
        }
        
        // Clear message if 1 second has passed
        if (showing_message && (GetTickCount() - message_start_time) >= 1000) {
            // Clear the message line
            gotoxy(2, height - 2);
            for (int i = 0; i < width - 10; i++) {
                printf(" ");
            }
            // Redraw command prompt
            draw_command_prompt(width, height);
            showing_message = 0;
        }
        
        // Position cursor (only if not showing a message, to avoid overwriting)
        if (!showing_message) {
            prompt_y = height - 3;
            prompt_x = 2 + 8 + cursor_pos;
            gotoxy(prompt_x, prompt_y);
        }
        
        if (_kbhit()) {
            ch = _getch();
            
            if (ch == '\r') {  // Enter
                // Clear the command line
                gotoxy(2 + 8, height - 3);
                for (int i = 0; i < cursor_pos; i++) {
                    printf(" ");
                }
                
                // Show command executed
                gotoxy(2, height - 2);
                printf("Command: %s", command_buffer);
                
                // Reset buffer
                command_buffer[0] = '\0';
                cursor_pos = 0;
                gotoxy(2 + 8, height - 3);
                // Start message timer
                showing_message = 1;
                message_start_time = GetTickCount();
            }
            else if (ch == '\b') {  // Backspace
                if (cursor_pos > 0 && !showing_message) {
                    cursor_pos--;
                    command_buffer[cursor_pos] = '\0';
                    gotoxy(2 + 8 + cursor_pos, height - 3);
                    printf(" ");
                    gotoxy(2 + 8 + cursor_pos, height - 3);
                }
            }
            else if (ch == 27) {  // ESC
                running = 0;
            }
            else if (ch >= 32 && ch <= 126 && !showing_message) {  // Printable
                if (cursor_pos < 255) {
                    command_buffer[cursor_pos] = ch;
                    printf("%c", ch);
                    cursor_pos++;
                    command_buffer[cursor_pos] = '\0';
                }
            }
        }
        Sleep(50);
    }
	show_cursor();    
    gotoxy(0, height - 1);
    printf("\nGoodbye!\n");
    return 0;
}
