#ifndef MODE_H
#define MODE_H

typedef enum {
    HOME = 0,
    CALENDAR,
    TASK,
    FOCUS,
    CLOCK,
    LOCK,
    HISTORY
} Mode;

// ==================== GLOBAL VARIABLES ====================
extern int current_mode;
extern int previous_mode;
void mode_display(void);
#endif