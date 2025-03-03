/**
 * parser.h - Command parsing functionality
 */

#ifndef PARSER_H
#define PARSER_H

#include "common.h"

/* Command line structure */
struct command_line {
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};

/* Function declarations */
struct command_line *parse_input();
void free_command(struct command_line *command);

#endif /* PARSER_H */
