#include "cleanup.h"
#include "terminal.h"
#include "cursor.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// ==================== CLEANUP FUNCTIONS ====================
void cleanup() {
    trm_restore_original_color();
    cursor_show();
    trm_gotoxy(0, term.height);
    printf("\n");
}

void cleanup_signal_handler(int sig) {
    if (sig == SIGINT) {
        cleanup();
        exit(0);
    }
}

void cleanup_setup() {
    atexit(cleanup);
    signal(SIGINT, cleanup_signal_handler);
}