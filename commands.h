/**
 * commands.h - Command execution functionality
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <sys/types.h>
#include "parser.h"
#include "bg_process.h"

/* Function declarations */
int execute_command(
    struct command_line *command,
    int *exit_status,
    bool *was_terminated,
    int *signal_number,
    struct bg_process_node **bg_processes_list,
    bool foreground_only
);

int change_directory(int argc, char **argv);

void update_status(
    int child_status,
    int *exit_status,
    bool *terminated,
    int *signal_number
);

void print_status(int exit_status, bool terminated, int signal_number);

#endif /* COMMANDS_H */
