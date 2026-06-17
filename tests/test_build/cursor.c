#include "cursor.h"
#include "terminal.h"
#include "command.h"
#include <windows.h>
#include <string.h>

// --- command line and cursor
Cursor cursor = {0, 0, 1, 0, 0};

// ==================== CURSOR FUNCTIONS ====================
void cursor_show() 
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void cursor_hide() 
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void cursor_erase_old() 
{
    int screen_pos = cursor.prev_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;

    trm_gotoxy(x, y);

    if (cursor.prev_pos < strlen(cmdline.buffer))
        putchar(cmdline.buffer[cursor.prev_pos]);
    else
        putchar(' ');
}

void cursor_draw_manual() 
{
    int screen_pos =
        cmdline.cursor_pos - cmdline.scroll_offset;

    if (screen_pos < 0) return;

    int visible_width = term.width - 4 - term.prompt_len;

    if (screen_pos >= visible_width) return;

    int x = 2 + term.prompt_len + screen_pos;
    int y = term.height - 3;
    trm_gotoxy(x, y);
    putchar('_');
    cursor.prev_pos = cmdline.cursor_pos;
    fflush(stdout);
}

void cursor_update_blink() 
{
    DWORD now = GetTickCount();
    if (!(now - cursor.last_blink >= 500)) 
    {
        return; // not enough time has passed
    }
    cursor.last_blink = now;
    if (cursor.visible) 
    {
        cursor_erase_old();    // hide cursor
        cursor.visible = 0;
    }
    else 
    {
        cursor_draw_manual();  // show cursor
        cursor.visible = 1;
    }
}

void cursor_position() {
    if (!cmdline.showing_message) {
        trm_gotoxy(2 + term.prompt_len + cmdline.cursor_pos, term.height - 3);
    }
}