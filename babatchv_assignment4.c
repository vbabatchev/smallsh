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
#include <unistd.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512
#define EXIT_CMD "exit"
#define CD_CMD "cd"

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
 * @return: A pointer to the command_line structure containing the
 *      parsed input.
 */
struct command_line *parse_input()
{
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
 * Changes the current working directory to the specified path.
 *
 * @param int argc: The number of arguments passed to the command.
 * @param char **argv: The array of arguments passed to the command.
 *
 * @return: 0 on success, -1 on failure.
 */
 int change_directory(int argc, char **argv) {
    char *target_dir;

    // If no arguments are provided, change to the home directory
    if (argc ==  1) {
        // Get HOME environment variable
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            perror("HOME not set");
            return -1;
        }
    } else {
        // Use the provided directory
        target_dir = argv[1];
    }

    // Change directory
    if (chdir(target_dir) != 0) {
        perror("Change directory failed");
        return -1;
    }
    return 0;
}

/**
 * Executes a parsed command.
 *
 * @param struct command_line *command: A pointer to the parsed command
 *      line structure or NULL.
 *
 * @return 0 to continue running, 1 to exit normally
 */
int execute_command(struct command_line *command) {
    // Handle NULL command or empty command
    if (command == NULL || command->argc == 0) {
        return 0; // Continue running the shell
    }

    // Check for 'exit' command
    if (strcmp(command->argv[0], EXIT_CMD) == 0) {
        // TODO: Clean up all background processes
        return 1; // Exit the shell
    }

    // Check for 'cd' command
    if (strcmp(command->argv[0], CD_CMD) == 0) {
        change_directory(command->argc, command->argv);
    }

    // TODO: Implement other commands
    return 0; // Continue running the shell
}

int main()
{
	struct command_line *curr_command;
	int shell_status = 0; // Shell status code

	while(shell_status == 0) { // Continue running while shell_status is 0
		curr_command = parse_input();

		// Handle parsing error
		if (curr_command == NULL) {
		    perror("Failed to parse input");
            continue;
        }

		// Execute the command and get the shell status
		shell_status = execute_command(curr_command);

		// Free allocated memory
		for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
		if (curr_command ->input_file != NULL) {
            free(curr_command->input_file);
        }
		if (curr_command ->output_file != NULL) {
            free(curr_command->output_file);
        }
		free(curr_command);
	}
	return EXIT_SUCCESS;
}
