/**
 * common.h - Common definitions for smallsh
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

/* Constants */
#define INPUT_LENGTH 2048
#define MAX_ARGS 512
#define COMMENT_FLAG '#'
#define EXIT_CMD "exit"
#define CD_CMD "cd"
#define STATUS_CMD "status"

/* Global variable declaration */
extern bool foreground_only; // Flag for foreground-only mode

#endif /* COMMON_H */
