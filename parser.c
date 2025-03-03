/**
 * parser.c - Implementation of command parsing functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

/**
 * Parses the user input and returns a command_line structure.
 *
 * @return: A pointer to the command_line structure containing the
 *     parsed input.
 */
struct command_line *parse_input() {
    char input[INPUT_LENGTH];
    struct command_line *curr_command = (struct command_line *)calloc(
       1, sizeof(struct command_line)
    );

    if (curr_command == NULL) {
        perror("Memory allocation for current command failed");
        return NULL;
    }

    // Get input
    printf(": ");
    fflush(stdout);
    fgets(input, INPUT_LENGTH, stdin);

    // Tokenize the input
    char *token = strtok(input, " \n");
    while(token){
        if(!strcmp(token,"<")){
            curr_command->input_file = strdup(strtok(NULL," \n"));
        } else if(!strcmp(token,">")){
            curr_command->output_file = strdup(strtok(NULL," \n"));
        } else if(!strcmp(token,"&")){
            curr_command->is_bg = true;
        } else{
            curr_command->argv[curr_command->argc++] = strdup(token);
        }
        token=strtok(NULL," \n");
    }
    return curr_command;
}

/**
 * Frees memory allocated for a command_line structure.
 *
 * @param command: The command_line structure to free.
 */
void free_command(struct command_line *command) {
    if (command == NULL) {
        return;
    }

    // Free argument strings
    for (int i = 0; i < command->argc; i++) {
        free(command->argv[i]);
    }

    // Free redirection filenames
    if (command->input_file != NULL) {
        free(command->input_file);
    }
    if (command->output_file != NULL) {
        free(command->output_file);
    }

    // Free the command structure itself
    free(command);
}
