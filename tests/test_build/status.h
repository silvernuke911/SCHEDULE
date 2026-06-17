#ifndef STATUS_H
#define STATUS_H

// ==================== STATUS LINE ========================
void status_clear(void);
void status_draw(const char *msg);
void status_set(const char *tag, const char *msg);

#endif