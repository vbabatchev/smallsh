/**
 * signals.c - Signal handling implementation
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "signals.h"
#include "common.h"

/* Global variable definition */
bool foreground_only = false; // Initially, background commands are allowed

/**
 * Signal handler for SIGTSTP (Ctrl-Z)
 *
 * @param signo: The signal number
 */
void handle_SIGTSTP(int signo) {
    char* enter_message = "\nEntering foreground-only mode (& is now ignored)\n: ";
    char* exit_message = "\nExiting foreground-only mode\n: ";

    // Toggle the foreground-only mode
    foreground_only = !foreground_only;

    // Write the message to stdout
    if (foreground_only) {
        write(STDOUT_FILENO, enter_message, strlen(enter_message));
    } else {
        write(STDOUT_FILENO, exit_message, strlen(exit_message));
    }
}

/**
 * Set up the signal handlers for SIGINT and SIGTSTP
 *
 * @param is_shell: A flag indicating if the shell is running in the foreground
 * @param is_background: A flag indicating if the command is a background process
 */
void setup_signal_handlers(bool is_shell, bool is_background) {
    // Set up signal handler for SIGINT
    struct sigaction SIGINT_action = {0};

    if (is_shell || is_background) {
        SIGINT_action.sa_handler = SIG_IGN; // Ignore SIGINT
    } else {
        SIGINT_action.sa_handler = SIG_DFL; // Default action for SIGINT
    }

    // Install the signal handler
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Set up signal handler for SIGTSTP
    struct sigaction SIGTSTP_action = {0};

    if (is_shell) {
        // Shell process handles SIGTSTP
        SIGTSTP_action.sa_handler = handle_SIGTSTP;
        // Block all signals during execution of the handler
        sigfillset(&SIGTSTP_action.sa_mask);
        // Automatically restart system calls if interrupted
        SIGTSTP_action.sa_flags = SA_RESTART;
    } else {
        // All child processes ignore SIGTSTP
        SIGTSTP_action.sa_handler = SIG_IGN;
    }

    // Install the SIGTSTP handler
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}
