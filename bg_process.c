/**
 * bg_process.c - Background process management functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "bg_process.h"

/**
 * Add a background process to the linked list.
 *
 * @param head: A pointer to a pointer to the head of the background process list
 * @param pid: The process ID of the background process
 * @return: 0 on success, -1 on failure
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
 * Check for completed background processes
 *
 * @param head: A pointer to a pointer to the head of the background process list
 */
void check_bg_processes(struct bg_process_node **head) {
    struct bg_process_node *current = *head;
    struct bg_process_node *prev = NULL;

    // Iterate through the linked list of background processes
    while (current != NULL) {
        int child_status;
        int signal_number = 0;

        // Check if the process is still active
        pid_t result = waitpid(current->pid, &child_status, WNOHANG);
        if (result == -1) {
            perror("waitpid() failed");
        } else if (result > 0) {
            // Process completed
            current->is_active = false;

            // Check if the process was terminated by a signal
            if (WIFSIGNALED(child_status)) {
                signal_number = WTERMSIG(child_status);
                printf("background pid %d is done: terminated by signal %d\n",
                        current->pid, signal_number);
            } else {
                printf("background pid %d is done: exit value %d\n",
                        current->pid, WEXITSTATUS(child_status));
            }
            fflush(stdout);

            // Remove the node from the linked list
            if (prev == NULL) {
                *head = current->next;
                free(current);
                current = *head;
            } else {
                prev->next = current->next;
                free(current);
                current = prev->next;
            }
            continue;
        }
        prev = current;
        current = current->next;
    }
}

/**
 * Clean up all background processes
 *
 * @param head: A pointer to a pointer to the head of the background process list
 */
void cleanup_bg_processes(struct bg_process_node **head) {
    struct bg_process_node *current = *head;
    struct bg_process_node *next;

    // Terminate all active processes
    while (current != NULL) {
        if (current->is_active) {
            kill(current->pid, SIGTERM);
        }
        current = current->next;
    }

    sleep(1); // Give processes time to terminate

    // Force kill any remaining processes
    current = *head;
    while (current != NULL) {
        if (current->is_active && kill(current->pid, 0) == 0) {
            kill(current->pid, SIGKILL);
        }
        current = current->next;
    }

    // Clean up zombie processes
    while (waitpid(-1, NULL, WNOHANG) > 0);

    // Free the linked list
    current = *head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    *head = NULL;
}
