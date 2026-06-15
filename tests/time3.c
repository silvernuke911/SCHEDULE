#include <stdio.h>
#include <time.h>
#include <windows.h>

#define MILITARY 0
#define CIVILIAN 1

// Big digits (5x3 each)
const char *digits[10][5] = {
    // 0
    {"█████",
     "█   █",
     "█   █",
     "█   █",
     "█████"},
    // 1
    {"  █  ",
     " ██  ",
     "  █  ",
     "  █  ",
     " ███ "},
    // 2
    {"█████",
     "    █",
     "█████",
     "█    ",
     "█████"},
    // 3
    {"█████",
     "    █",
     " ████",
     "    █",
     "█████"},
    // 4
    {"█   █",
     "█   █",
     "█████",
     "    █",
     "    █"},
    // 5
    {"█████",
     "█    ",
     "█████",
     "    █",
     "█████"},
    // 6
    {"█████",
     "█    ",
     "█████",
     "█   █",
     "█████"},
    // 7
    {"█████",
     "    █",
     "   █ ",
     "  █  ",
     "  █  "},
    // 8
    {"█████",
     "█   █",
     "█████",
     "█   █",
     "█████"},
    // 9
    {"█████",
     "█   █",
     "█████",
     "    █",
     "█████"}
};

// Colon for between digits
const char *colon[5] = {
    "     ",
    "  █  ",
    "     ",
    "  █  ",
    "     "
};

void print_big_digit(int digit, int row) {
    printf("%s", digits[digit][row]);
}

void print_colon(int row) {
    printf("%s", colon[row]);
}

void hide_cursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void print_centered_date(int y_pos) {
    time_t rawtime;
    struct tm *timeinfo;
    char date_str[100];
    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // Format date like "Jun 12, 2026"
    sprintf(date_str, "%s %d, %d", 
            months[timeinfo->tm_mon], 
            timeinfo->tm_mday, 
            timeinfo->tm_year + 1900);
    
    // Calculate centered position (assuming 80 columns)
    int len = strlen(date_str);
    int padding = (56 - len) / 2;
    
    gotoxy(padding, y_pos);
    printf("%s", date_str);
}

void print_time(int mode) {
    time_t rawtime;
    struct tm *timeinfo;
    SetConsoleOutputCP(CP_UTF8);
    int h, hf, h1, h2, m1, m2, s1, s2;
    int x0 = 1;
    int y0 = 1;
    
    hide_cursor();
    system("cls");
    
    while (1) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        h = timeinfo->tm_hour;
        if (mode == CIVILIAN) {
            hf = h % 12;
            if (hf == 0) hf = 12;  // 12 AM/PM
        } else {
            hf = h;
        }
        
        // Split into individual digits
        h1 = hf / 10;
        h2 = hf % 10;
        m1 = timeinfo->tm_min / 10;
        m2 = timeinfo->tm_min % 10;
        s1 = timeinfo->tm_sec / 10;
        s2 = timeinfo->tm_sec % 10;
        
        // Print the big clock (5 rows)
        for (int row = 0; row < 5; row++) {
            gotoxy(x0, y0 + row);
            
            // Hour tens
            if (h1 == 0 && h2 != 0) {
                printf("     ");
            } else {
                print_big_digit(h1, row);
            }
            printf("  ");
            
            // Hour ones
            print_big_digit(h2, row);
            printf("  ");
            
            // Colon
            print_colon(row);
            printf("  ");
            
            // Minute tens
            print_big_digit(m1, row);
            printf("  ");
            
            // Minute ones
            print_big_digit(m2, row);
            printf("  ");
            
            // Colon
            print_colon(row);
            printf("  ");
            
            // Second tens
            print_big_digit(s1, row);
            printf("  ");
            
            // Second ones
            print_big_digit(s2, row);
            
            // Print AM/PM if civilian time
            if (mode == CIVILIAN && row == 2) {
                if (h >= 12) {
                    printf("  PM");
                } else {
                    printf("  AM");
                }
            }
        }
        
        // Print centered date at bottom (row 9)
        print_centered_date(7);
        
        Sleep(1000);
    }
}

int main() {
    print_time(CIVILIAN);
    return 0;
}
