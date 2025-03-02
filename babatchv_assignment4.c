/**
 * Program Name: babatchv_assignment4.c
 * Author: Velislav Babatchev
 *
 * This program provides a simple shell interface that allows users to
 * execute commands, redirect input and output, and run commands in the
 * background.
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512
#define COMMENT_FLAG '#'
#define EXIT_CMD "exit"
#define CD_CMD "cd"
#define STATUS_CMD "status"

struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

/**
 * Structure to track background processes
 */
struct bg_process_node {
    pid_t pid;
    bool is_active;
    struct bg_process_node *next;
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
 * Updates the exit status based on the child process's termination
 * status.
 *
 * @param int child_status: The termination status of the child process.
 * @param int *exit_status: A pointer to an integer to store the exit
 *      status of the executed command.
 * @param bool *terminated: A pointer to a boolean flag to indicate
 *     if the command was terminated by a signal.
 * @param int *signal_number: A pointer to an integer to store the
 *      signal number that terminated the command.
 */
 void update_status(
    int child_status,
    int *exit_status,
    bool *terminated,
    int *signal_number
 ) {
    if (WIFEXITED(child_status)) {
        *exit_status = WEXITSTATUS(child_status);
    } else if (WIFSIGNALED(child_status)) {
        *signal_number = WTERMSIG(child_status);
        *terminated = true;
    }
}
/**
 * Prints the exit status or terminating signal of the last executed
 * foreground command.
 *
 * @param int exit_status: The exit status of the last executed
 *      foreground command.
 * @param bool terminated: A flag indicating if the last foreground
 *      command was terminated by a signal.
 * @param int signal_number: The signal number that terminated the last
 *      foreground command.
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
 * Redirects input and output for the command.
 *
 * @param struct command_line *command: A pointer to the command_line
 *      structure containing the command to be executed.
 * @param int *exit_status: A pointer to an integer to store the exit
 *      status of the executed command.
 * @param bool is_bg: A flag indicating if the command is a background
 *      process.
 *
 * @return: 0 on success, -1 on failure.
 */
int redirect(struct command_line *command, int *exit_status, bool is_bg) {
    // Redirect input
    if (command->input_file != NULL) {
        // Open the input file for reading
        int input_fd = open(command->input_file, O_RDONLY);
        if (input_fd == -1) {
            fprintf(stderr, "cannot open %s for input\n", command->input_file);
            fflush(stderr);
            *exit_status = EXIT_FAILURE;
            return -1;
        }

        // Redirect standard input to the input file
        if (dup2(input_fd, 0) == -1) {
            perror("source dup2()");
            close(input_fd);
            *exit_status = EXIT_FAILURE;
            return -1;
        };

        // Close the input file descriptor
        close(input_fd);
    } else if (is_bg) {
        // If it's a background process and no input file is specified,
        // redirect standard input to /dev/null
        int null_fd = open("/dev/null", O_RDONLY);
        if (null_fd == -1) {
            perror("cannot open /dev/null for input");
            *exit_status = EXIT_FAILURE;
            return -1;
        }

        // Redirect standard input to /dev/null
        if (dup2(null_fd, 0) == -1) {
            perror("source dup2()");
            close(null_fd);
            *exit_status = EXIT_FAILURE;
            return -1;
        };

        // Close the null file descriptor
        close(null_fd);
    }

    // Redirect output
    if (command->output_file != NULL) {
        // Open the output file for writing
        int output_fd = open(
            command->output_file,
            O_WRONLY | O_CREAT | O_TRUNC,
            0644
        );
        if (output_fd == -1) {
            fprintf(stderr, "cannot open %s for output\n", command->output_file);
            fflush(stderr);
            *exit_status = EXIT_FAILURE;
            return -1;
        }

        // Redirect standard output to the output file
        if (dup2(output_fd, 1) == -1) {
            perror("target dup2()");
            close(output_fd);
            *exit_status = EXIT_FAILURE;
            return -1;
        };

        // Close the output file descriptor
        close(output_fd);
    } else if (is_bg) {
        // If it's a background process and no output file is specified,
        // redirect standard output to /dev/null
        int null_fd = open("/dev/null", O_WRONLY);
        if (null_fd == -1) {
            perror("cannot open /dev/null for output");
            *exit_status = EXIT_FAILURE;
            return -1;
        }

        // Redirect standard output to /dev/null
        if (dup2(null_fd, 1) == -1) {
            perror("target dup2()");
            close(null_fd);
            *exit_status = EXIT_FAILURE;
            return -1;
        };

        // Close the null file descriptor
        close(null_fd);
    }
    return 0;
}

/**
 * Add a background process to the linked list.
 *
 * @param struct bg_process_node **head: A pointer to a pointer to the
 *      head of the background process list.
 * @param pid_t pid: The process ID of the background process.
 *
 * @return: 0 on success, -1 on failure.
 */
int add_bg_process(struct bg_process_node **head, pid_t pid) {
    // Allocate memory for a new background process node
    struct bg_process_node *new_bg_process = (struct bg_process_node *)malloc(
        sizeof(struct bg_process_node)
    );
    // Check if memory allocation was successful
    if (new_bg_process == NULL) {
        perror("Memory allocation for background process node failed");
        return -1;
    }

    // Initialize the new node
    new_bg_process->pid = pid;
    new_bg_process->is_active = true;
    new_bg_process->next = *head; // Insert at the beginning of the list
    *head = new_bg_process;

    return 0;
}

/**
 * Executes a parsed command.
 *
 * @param struct command_line *command: A pointer to the parsed command
 *      line structure or NULL.
 * @param int *exit_status: A pointer to an integer to store the
 *      exit status of the last foreground process.
 * @param bool *was_terminated: A pointer to a boolean flag to indicate
 *     if the last foreground process was terminated by a signal.
 * @param int *signal_number: A pointer to an integer to store the
 *     signal number that terminated the command.
 * @param struct bg_process_node **bg_processes_list: A pointer to a
 *     pointer to the head of the background process list.
 *
 * @return 0 to continue running, 1 to exit normally
 */
int execute_command(
    struct command_line *command,
    int *exit_status,
    bool *was_terminated,
    int *signal_number,
    struct bg_process_node **bg_processes_list
) {
    int child_status;
    pid_t child_pid = -5;

    // Handle NULL command, empty command, blank line, or comment
    if ((command == NULL) ||
        (command->argc == 0) ||
        (strcmp(command->argv[0], "\0") == 0) ||
        (command->argv[0][0] == '#')
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

    // Other commands
    child_pid = fork();
    switch (child_pid) {
		case -1:
			perror("fork() failed");
			break;
		case 0:
			// Child process
			// Redirect input and output if specified
			if (redirect(command, exit_status, command->is_bg) != 0) {
			    exit(EXIT_FAILURE);
            }
			// Execute the command
 			execvp(command->argv[0], command->argv);
 			// If execvp fails, print error message and set exit status
 			perror("execvp() failed");
 			exit(EXIT_FAILURE);
			break;
		default:
		    // Parent process
			// Wait for child process to finish it it's a foreground process
            if (!command->is_bg) {
    		    child_pid = waitpid(child_pid, &child_status, 0);
    			update_status(
                    child_status,
    				exit_status,
                    was_terminated,
    				signal_number
                );
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

/**
 * Check for completed background processes, print their status, and
 *      free them from memory.
 *
 * @param struct bg_process_node **head: A pointer to a pointer to the
 *      head of the background process list.
 */
void check_bg_processes(struct bg_process_node **head) {
    struct bg_process_node *current = *head;
    struct bg_process_node *prev = NULL;

    // Iterate through the linked list of background processes
    while (current != NULL) {
        int child_status;
        bool was_terminated = false;
        int signal_number = 0;

        // Check if the process is still active
        pid_t result = waitpid(current->pid, &child_status, WNOHANG);
        if (result == -1) {
            perror("waitpid() failed");
        } else if (result > 0) {
            // Process completed successfully
            current->is_active = false; // Mark the process as inactive
            // Check if the process was terminated by a signal
            if (WIFSIGNALED(child_status)) {
                was_terminated = true;
                signal_number = WTERMSIG(child_status);
            }
            printf("background pid %d is done: ", current->pid);
            print_status(child_status, was_terminated, signal_number);

            // Remove the node from the linked list
            if (prev == NULL) {
                *head = current->next; // Update head of the list
                free(current);
                current = *head; // Move to the next node
            } else {
                prev->next = current->next; // Bypass the current node
                free(current);
                current = prev->next; // Move to the next node
            }
            continue; // Continue to check the next process
        }
        prev = current;
        current = current->next;
    }
}

/**
 * Clean up all background processes and free the linked list.
 *
 * @param struct bg_process_node **head: A pointer to a pointer to the
 *      head of the background process list.
 */
void cleanup_bg_processes(struct bg_process_node **head) {
    struct bg_process_node *current = *head;
    struct bg_process_node *next;

    // Iterate through the linked list and terminate all active processes
    while (current != NULL) {
        if (current->is_active) {
            kill(current->pid, SIGTERM); // Terminate the process
        }
        current = current->next; // Move to the next node
    }

    // Pause to terminate
    sleep(1);

    // Force kill any remaining processes
    current = *head;
    while (current != NULL) {
        if (current->is_active) {
            // Check if process is still running
            if (kill(current->pid, 0) == 0) {
                kill(current->pid, SIGKILL); // Force kill the process
            }
        }
        current = current->next; // Move to the next node
    }

    // Clean up zombie processes
    while (waitpid(-1, NULL, WNOHANG) > 0);

    // Free the linked list
    current = *head;
    while (current != NULL) {
        next = current->next; // Store the next node
        free(current); // Free the current node
        current = next; // Move to the next node
    }

    *head = NULL; // Set the head of the list to NULL
}

int main() {
	struct command_line *curr_command;
	int shell_status = 0; // Shell status code
	int exit_status = EXIT_SUCCESS; // Last foreground exit status
	int signal_number = 0; // Signal number for terminated processes
	bool was_terminated = false; // Flag for terminated processes

	// Initialize background process list
	struct bg_process_node *bg_processes_list = NULL;

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
		    &bg_processes_list
		);

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

	// Free all background processes
	cleanup_bg_processes(&bg_processes_list);

	return EXIT_SUCCESS;
}
