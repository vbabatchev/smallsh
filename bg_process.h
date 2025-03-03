/**
 * bg_process.h - Background process management
 */

#ifndef BG_PROCESS_H
#define BG_PROCESS_H

#include <sys/types.h>
#include <stdbool.h>

/**
 * Structure to track background processes
 */
struct bg_process_node {
    pid_t pid;
    bool is_active;
    struct bg_process_node *next;
};

/* Function declarations */
int add_bg_process(struct bg_process_node **head, pid_t pid);
void check_bg_processes(struct bg_process_node **head);
void cleanup_bg_processes(struct bg_process_node **head);

#endif /* BG_PROCESS_H */
