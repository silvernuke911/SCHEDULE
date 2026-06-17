#ifndef INPUT_H
#define INPUT_H

// ==================== INPUT HANDLING ====================
int input_handle_keyboard();
int input_handle_regular_keys(int ch);
int input_handle_extended_keys(int ch);
void input_handle_enter();
static void handle_up_arrow(void);
static void handle_down_arrow(void);
static void handle_left_arrow(void);
static void handle_right_arrow(void);
#endif