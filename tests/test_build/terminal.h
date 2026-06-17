#ifndef TERMINAL_H
#define TERMINAL_H

#include <windows.h>
#include <stdio.h>

// ==================== STRUCT DEFINITIONS ====================
typedef struct {
    int width;
    int height;
    int old_width;
    int old_height;
    int sep_bar;
	int prompt_len;
} Terminal;

// ==================== GLOBAL VARIABLES ====================
extern Terminal term;

// ==================== TERMINAL FUNCTIONS ====================
void get_terminal_size(int *width, int *height);
void trm_gotoxy(int x, int y);
void trm_save_original_color();
void trm_set_console_color(int foreground, int background);
void trm_restore_original_color();
void trm_set_white_text();
void trm_draw_border(int width, int height, int sep_bar);
void trm_draw_command_prompt();
void trm_clear_screen();
void trm_redraw_border_if_changed();

#endif