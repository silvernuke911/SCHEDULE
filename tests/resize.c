#include <stdio.h>
#include <windows.h>
#include <conio.h>   // For getch()

int get_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int get_height() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

int check_terminal_size(int required_width, int required_height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    return (width >= required_width && height >= required_height);
}

int main() {
    // Using CP437 box characters
    // 201 = ╔, 205 = ═, 187 = ╗
    // 186 = ║, 200 = ╚, 188 = ╝
    
    printf("\n");
    printf("%c", 201);  // ╔
    for (int i = 0; i < 60; i++) printf("%c", 205);  // ═
    printf("%c\n", 187);  // ╗
    
    printf("%c  TERMINAL SIZE REQUIRED  %c\n", 186, 186);  // ║ ║
    
    printf("%c", 200);  // ╚
    for (int i = 0; i < 60; i++) printf("%c", 205);  // ═
    printf("%c\n", 188);  // ╝
    
    return 0;
}
