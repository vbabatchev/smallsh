/**
 * commands.c - Implementation of command execution functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "commands.h"
#include "common.h"
#include "io.h"
#include "signals.h"

/**
 * Changes the current working directory to the specified path.
 *
 * @param argc: The number of arguments passed to the command.
 * @param argv: The array of arguments passed to the command.
 *
 * @return: 0 on success, -1 on failure.
 */
int change_directory(int argc, char **argv) {
    char *target_dir;

    // If no arguments are provided, change to the home directory
    if (argc == 1) {
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
 * Updates the exit status based on the child process's termination
 * status.
 *
 * @param child_status: The termination status of the child process.
 * @param exit_status: A pointer to an integer to store the exit
 *     status of the executed command.
 * @param terminated: A pointer to a boolean flag to indicate
 *     if the command was terminated by a signal.
 * @param signal_number: A pointer to an integer to store the
 *     signal number that terminated the command.
 */
void update_status(
    int child_status,
    int *exit_status,
    bool *terminated,
    int *signal_number
) {
    if (WIFEXITED(child_status)) {
        *exit_status = WEXITSTATUS(child_status);
        *terminated = false;
    } else if (WIFSIGNALED(child_status)) {
        *signal_number = WTERMSIG(child_status);
        *terminated = true;
    }
}

/**
 * Prints the exit status or terminating signal of the last executed
 * foreground command.
 *
 * @param exit_status: The exit status of the last executed
 *     foreground command.
 * @param terminated: A flag indicating if the last foreground
 *     command was terminated by a signal.
 * @param signal_number: The signal number that terminated the last
 *     foreground command.
 */
void print_status(int exit_status, bool terminated, int signal_number) {
    if (terminated) {
        printf("terminated by signal %d\n", signal_number);
    } else {
        printf("exit value %d\n", exit_status);
    }
    fflush(stdout);
}

/**
 * Executes a parsed command.
 *
 * @param command: A pointer to the parsed command line structure or NULL.
 * @param exit_status: A pointer to an integer to store the
 *     exit status of the last foreground process.
 * @param was_terminated: A pointer to a boolean flag to indicate
 *     if the last foreground process was terminated by a signal.
 * @param signal_number: A pointer to an integer to store the
 *     signal number that terminated the command.
 * @param bg_processes_list: A pointer to a pointer to the head of the
 *     background process list.
 *
 * @return 0 to continue running, 1 to exit normally
 */
int execute_command(
    struct command_line *command,
    int *exit_status,
    bool *was_terminated,
    int *signal_number,
    struct bg_process_node **bg_processes_list,
    bool foreground_only
) {
    int child_status;
    pid_t child_pid = -5;

    // Handle NULL command, empty command, blank line, or comment
    if ((command == NULL) ||
        (command->argc == 0) ||
        (strcmp(command->argv[0], "\0") == 0) ||
        (command->argv[0][0] == COMMENT_FLAG)
    ) {
        return 0; // Continue running the shell
    }

    // Check for 'exit' command
    if (strcmp(command->argv[0], EXIT_CMD) == 0) {
        return 1; // Exit the shell
    }

    // Check for 'cd' command
    if (strcmp(command->argv[0], CD_CMD) == 0) {
        change_directory(command->argc, command->argv);
        return 0; // Continue running the shell
    }

    // Check for 'status' command
    if (strcmp(command->argv[0], STATUS_CMD) == 0) {
        print_status(*exit_status, *was_terminated, *signal_number);
        return 0; // Continue running the shell
    }

    // If foreground-only mode is enabled, ignore background processes
    if (foreground_only) {
        command->is_bg = false;
    }

    // Other commands
    child_pid = fork();
    switch (child_pid) {
        case -1:
            perror("fork() failed");
            break;
        case 0:
            // Child process

            // Set up signal handler for child process
            setup_signal_handlers(false, command->is_bg, NULL);

            // Redirect input and output if specified
            if (redirect(command, exit_status, command->is_bg) != 0) {
                exit(EXIT_FAILURE);
            }
            // Execute the command
            execvp(command->argv[0], command->argv);
            // If execvp fails, print error message and exit
            perror("execvp() failed");
            exit(EXIT_FAILURE);
            break;
        default:
            // Parent process
            // Wait for child process to finish if it's a foreground process
            if (!command->is_bg) {
                child_pid = waitpid(child_pid, &child_status, 0);
                update_status(
                    child_status,
                    exit_status,
                    was_terminated,
                    signal_number
                );

                // Check if the child process was terminated by a signal
                if (*was_terminated) {
                    printf("terminated by signal %d\n", *signal_number);
                    fflush(stdout);
                }
            } else {
                // Background process
                printf("background pid is %d\n", child_pid);
                fflush(stdout);
                // Add the background process to the list
                if (add_bg_process(bg_processes_list, child_pid) == -1) {
                    fprintf(stderr, "Failed to add background process\n");
                    fflush(stderr);
                }
            }
            break;
    }
    return 0; // Continue running the shell
}
