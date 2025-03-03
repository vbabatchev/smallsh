/**
 * io.c - Input/output redirection implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "io.h"

/**
 * Redirects input and output for the command
 *
 * @param command: A pointer to the command_line structure
 * @param exit_status: A pointer to an integer to store the exit status
 * @param is_bg: A flag indicating if the command is a background process
 * @return: 0 on success, -1 on failure
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
