#ifndef HISTORY_H
#define HISTORY_H

#include <windows.h>

#define CMD_BUF_SIZE 256
#define MAX_HISTORY 256

// ==================== STRUCT DEFINITIONS ====================
typedef struct {
    char history[MAX_HISTORY][CMD_BUF_SIZE];
    int count;
    int index;
} History;

// ==================== GLOBAL VARIABLES ====================
extern History history;

// ==================== HISTORY FUNCTIONS ====================
void history_load_command();
void input_add_history(void);

#endif