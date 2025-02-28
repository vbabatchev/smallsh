/**
 * Program Name: babatchv_assignment4.c
 * Author: Velislav Babatchev
 *
 * This program provides a simple shell interface that allows users to
 * execute commands, redirect input and output, and run commands in the
 * background.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512


struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

/**
 * Citation for the following function:
 * Date: 02/26/2025
 * Adapted from: sample_parser.c
 * Source URL:
 * https://canvas.oregonstate.edu/courses/1987883/assignments/9865611
 *
 * Parses the user input and returns a command_line structure.
 *
 * The function allocates memory for the command_line structure and its
 * members. The caller is responsible for freeing this memory.
 *
 * @return A pointer to the command_line structure containing the parsed
 *      input.
 */
struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *)calloc(
	   1, sizeof(struct command_line)
	);

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

int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();
		// Free allocated memory
		for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
		free(curr_command->input_file);
		free(curr_command->output_file);
		free(curr_command);
	}
	return EXIT_SUCCESS;
}
