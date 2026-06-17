#ifndef CURSOR_H
#define CURSOR_H

#include <windows.h>

// ==================== STRUCT DEFINITIONS ====================
typedef struct {
    int x;
    int y;
    int visible;
    int prev_pos;
    DWORD last_blink;
} Cursor;

// ==================== GLOBAL VARIABLES ====================
extern Cursor cursor;

// ==================== CURSOR FUNCTIONS ====================
void cursor_show();
void cursor_hide();
void cursor_erase_old();
void cursor_draw_manual();
void cursor_update_blink();
void cursor_position();

#endif