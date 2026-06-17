#include "terminal.h"
#include "command.h"
#include <windows.h>
#include <stdio.h>
#include "mode.h"

// --- terminal width
Terminal term = {0, 0, -1, -1, 4, 8};

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

void trm_clear_screen() {
    for (int x = 1; x < term.width - 1; x++) {
        for (int y = 1; y < term.height - term.sep_bar; y ++){
            trm_gotoxy(x,y);
            printf(" ");
        }
    }
}

void trm_redraw_border_if_changed() {
    get_terminal_size(&term.width, &term.height);
    
    if (term.width != term.old_width || term.height != term.old_height) {
        system("cls");
        trm_draw_border(term.width, term.height, term.sep_bar);
        trm_draw_command_prompt();
        previous_mode = -1;
        mode_display();
        cmd_redraw_buffer();
		trm_clear_screen();
        term.old_width = term.width;
        term.old_height = term.height;
    }
}