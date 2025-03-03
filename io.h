/**
 * io.h - Input/output redirection functionality
 */

#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include "parser.h"

/* Function declarations */
int redirect(struct command_line *command, int *exit_status, bool is_bg);

#endif /* IO_H */
