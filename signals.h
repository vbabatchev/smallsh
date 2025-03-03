/**
 * signals.h - Signal handling functionality
 */

#ifndef SIGNALS_H
#define SIGNALS_H

#include <stdbool.h>

/* Function declarations */
void handle_SIGTSTP(int signo);
void setup_signal_handlers(
    bool is_shell,
    bool is_background,
    bool *foreground_only
);

#endif /* SIGNALS_H */
