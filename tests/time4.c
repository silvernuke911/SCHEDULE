#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <string.h>

#define MILITARY 0
#define CIVILIAN 1

// Big digits (5x3 each) - each is 5 chars wide
const char *digits[10][5] = {
    {"█████", "█   █", "█   █", "█   █", "█████"},
    {"  █  ", " ██  ", "  █  ", "  █  ", " ███ "},
    {"█████", "    █", "█████", "█    ", "█████"},
    {"█████", "    █", " ████", "    █", "█████"},
    {"█   █", "█   █", "█████", "    █", "    █"},
    {"█████", "█    ", "█████", "    █", "█████"},
    {"█████", "█    ", "█████", "█   █", "█████"},
    {"█████", "    █", "   █ ", "  █  ", "  █  "},
    {"█████", "█   █", "█████", "█   █", "█████"},
    {"█████", "█   █", "█████", "    █", "█████"}
};

const char *colon[5] = {"     ", "  █  ", "     ", "  █  ", "     "};

// Store current digits being displayed
int current_h1 = -1, current_h2 = -1, current_m1 = -1, current_m2 = -1;
int current_s1 = -1, current_s2 = -1;
int current_ampm = -1;

void gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hide_cursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void show_cursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

// Update a single character at position
void update_char(int x, int y, char ch) {
    gotoxy(x, y);
    putchar(ch);
}

// Update a 5-char wide digit row at specific position
void update_digit_row(int digit, int row, int x, int y) {
    gotoxy(x, y + row);
    printf("%s", digits[digit][row]);
}

// Update only the parts that changed
void update_clock_if_changed(int mode) {
    time_t rawtime;
    struct tm *timeinfo;
    int h, hf, h1, h2, m1, m2, s1, s2;
    int x0 = 1, y0 = 1;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    h = timeinfo->tm_hour;
    if (mode == CIVILIAN) {
        hf = h % 12;
        if (hf == 0) hf = 12;
    } else {
        hf = h;
    }
    
    h1 = hf / 10;
    h2 = hf % 10;
    m1 = timeinfo->tm_min / 10;
    m2 = timeinfo->tm_min % 10;
    s1 = timeinfo->tm_sec / 10;
    s2 = timeinfo->tm_sec % 10;
    
    // Update hour tens
    if (h1 != current_h1) {
        for (int row = 0; row < 5; row++) {
            if (h1 == 0 && h2 != 0) {
                // Clear if leading zero
                gotoxy(x0, y0 + row);
                printf("     ");
            } else {
                update_digit_row(h1, row, x0, y0);
            }
        }
        current_h1 = h1;
    }
    
    // Update hour ones
    if (h2 != current_h2) {
        for (int row = 0; row < 5; row++) {
            update_digit_row(h2, row, x0 + 7, y0);
        }
        current_h2 = h2;
    }
    
    // Update minute tens
    if (m1 != current_m1) {
        for (int row = 0; row < 5; row++) {
            update_digit_row(m1, row, x0 + 20, y0);
        }
        current_m1 = m1;
    }
    
    // Update minute ones
    if (m2 != current_m2) {
        for (int row = 0; row < 5; row++) {
            update_digit_row(m2, row, x0 + 27, y0);
        }
        current_m2 = m2;
    }
    
    // Update second tens
    if (s1 != current_s1) {
        for (int row = 0; row < 5; row++) {
            update_digit_row(s1, row, x0 + 41, y0);
        }
        current_s1 = s1;
    }
    
    // Update second ones
    if (s2 != current_s2) {
        for (int row = 0; row < 5; row++) {
            update_digit_row(s2, row, x0 + 48, y0);
        }
        current_s2 = s2;
    }
    
    // Update AM/PM
    if (mode == CIVILIAN) {
        int ampm = (h >= 12);
        if (ampm != current_ampm) {
            gotoxy(x0 + 56, y0 + 2);
            printf(ampm ? "PM" : "AM");
            current_ampm = ampm;
        }
    }
}

void draw_static_parts(int mode) {
    int x0 = 1, y0 = 1;
    
    // Draw colons (they never change)
    for (int row = 0; row < 5; row++) {
        gotoxy(x0 + 14, y0 + row);
        printf("%s", colon[row]);
        
        gotoxy(x0 + 34, y0 + row);
        printf("%s", colon[row]);
    }
    
    // Draw initial clock values
    update_clock_if_changed(mode);
}

void print_centered_date(int y_pos) {
    time_t rawtime;
    struct tm *timeinfo;
    char date_str[100];
    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    sprintf(date_str, "%s %d, %d", 
            months[timeinfo->tm_mon], 
            timeinfo->tm_mday, 
            timeinfo->tm_year + 1900);
    
    int len = strlen(date_str);
    int padding = (56 - len) / 2;
    
    gotoxy(padding, y_pos);
    printf("%s", date_str);
}

int main() {
    int mode = CIVILIAN;
    char command_buffer[256] = "";
    int cursor_pos = 0;
    int ch;
    int running = 1;
    
    SetConsoleOutputCP(CP_UTF8);
    hide_cursor();
    system("cls");
    
    // Draw static elements
    draw_static_parts(mode);
    print_centered_date(7);
    
    // Draw command line
    gotoxy(0, 10);
    printf("tasks\\> ");
    
    while (running) {
        // Only update parts that changed
        update_clock_if_changed(mode);
		print_centered_date(7);
        // Position cursor for command input
        gotoxy(8 + cursor_pos, 10);
        
        // Handle keyboard input (non-blocking)
        if (_kbhit()) {
            ch = _getch();
            
            if (ch == '\r') {  // Enter
                if (strlen(command_buffer) > 0) {
                    gotoxy(0, 11);
                    printf("Command: %s", command_buffer);
                    // Clear buffer
                    command_buffer[0] = '\0';
                    cursor_pos = 0;
                    // Clear command line
                    gotoxy(0, 10);
                    printf("tasks\\>                                               ");
                    // Clear message after 1 sec
                    Sleep(1000);
                    gotoxy(0, 11);
                    printf("                                                       ");
                }
            }
            else if (ch == '\b') {  // Backspace
                if (cursor_pos > 0) {
                    cursor_pos--;
                    command_buffer[cursor_pos] = '\0';
                    gotoxy(8 + cursor_pos, 10);
                    printf(" ");
                    gotoxy(8 + cursor_pos, 10);
                }
            }
            else if (ch == 27) {  // ESC
                running = 0;
            }
            else if (ch >= 32 && ch <= 126) {  // Printable
                if (cursor_pos < 255) {
                    command_buffer[cursor_pos] = ch;
                    cursor_pos++;
                    command_buffer[cursor_pos] = '\0';
                    printf("%c", ch);
                }
            }
        }
        
        Sleep(50);  // Short delay to prevent CPU hogging
    }
    
    gotoxy(0, 12);
    show_cursor();
    printf("Goodbye!\n");
    return 0;
}
