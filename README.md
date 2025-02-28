# smallsh
A simple shell program written in C. This shell supports input/output redirection, background processes, and signal handling.

## Features
- Command execution
- Input/Output redirection
- Background processes
- Signal handling

## Usage
1. Clone the repository
2. Compile the program using the provided makefile
3. Run the program using the following command:
```
./smallsh
```

## Commands
- `exit` - Exits the shell
- `cd` - Changes the current working directory
- `status` - Prints the exit status of the last foreground process
- Any other command will be executed by the shell

## Input/Output Redirection
- Input redirection using `<`
- Output redirection using `>`

## Background Processes
- Background processes can be started by appending `&` to the end of a command
- The shell will print the PID of a background process when it starts

## Signal Handling
- The shell ignores `SIGINT` signals
- The shell will catch `SIGTSTP` signals and toggle foreground-only mode

## Author
- [Velislav Babatchev](https://github.com/vbabatchev)
