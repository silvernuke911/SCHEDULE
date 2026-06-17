#include "status.h"
#include "terminal.h"
#include <stdio.h>
#include <string.h>

// ===================== STATUS LINE ========================
void status_clear(void) {
    trm_gotoxy(2, term.height - 2);
    for (int i = 0; i < term.width - 4; i++)
        putchar(' ');
    fflush(stdout);
}

void status_draw(const char *msg) {
    trm_gotoxy(2, term.height - 2);
    int max_len = term.width - 4;
    
    // Print with field width - this automatically truncates and pads
    printf("%-*.*s", max_len, max_len, msg);
    
    fflush(stdout);
}

void status_set(const char *tag, const char *msg) {
    char buf[512];
    snprintf(buf, sizeof(buf), "[ %s ]: %s", tag, msg);
    status_draw(buf);
}
