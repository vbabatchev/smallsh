/**
 * Program Name: main.c
 * Author: Velislav Babatchev
 *
 * This program provides a simple shell interface that allows users to
 * execute commands, redirect input and output, and run commands in the
 * background.
 */

/* Standard library includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* Custom module includes */
#include "parser.h"
#include "commands.h"
#include "signals.h"
#include "bg_process.h"

/**
 * Main function for the shell program.
 *
 * @return: EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main() {
	struct command_line *curr_command;
	int shell_status = 0; // Shell status code
	int exit_status = EXIT_SUCCESS; // Last foreground exit status
	int signal_number = 0; // Signal number for terminated processes
	bool was_terminated = false; // Flag for terminated processes
	bool foreground_only = false; // Flag for foreground-only mode

	// Initialize background process list
	struct bg_process_node *bg_processes_list = NULL;

	// Set up signal handler for the shell
	setup_signal_handlers(true, false, &foreground_only);

	while(shell_status == 0) { // Continue running while shell_status is 0
	    // Check for completed background processes before each prompt
	    check_bg_processes(&bg_processes_list);

		// Display prompt and get user input
		curr_command = parse_input();

		// Handle parsing error or empty command
		if (curr_command == NULL) {
            continue;
        }

		// Execute the command and get the shell status
		shell_status = execute_command(
		    curr_command,
		    &exit_status,
			&was_terminated,
			&signal_number,
		    &bg_processes_list,
			foreground_only
		);

		// Free allocated memory
		free_command(curr_command);
	}

	// Free all background processes
	cleanup_bg_processes(&bg_processes_list);

	return EXIT_SUCCESS;
}
