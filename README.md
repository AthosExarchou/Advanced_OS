# README.md

## Project Overview
This project is an exercise for the Advanced Operating Systems course at Harokopio
University of Athens, Dept. of Informatics and Telematics. It implements a worker pool paradigm using processes and pipes for inter-process communication. The parent process distributes tasks to child processes, which execute them and respond with results. The program ensures proper synchronization using semaphores and supports a graceful shutdown on receiving termination signals (e.g., SIGINT or SIGTERM).

### Features
- Task distribution to child processes.
- Bidirectional communication using pipes.
- Semaphore-based synchronization.
- Graceful shutdown with resource cleanup.
- Dynamic handling of tasks and child processes.

### Prerequisites
- GCC compiler (for compiling C code).
- Linux environment (for pipe and semaphore usage).
- `make` (for building the project).

## How to run

### Compile and execute
To compile and run the program, use the provided Makefile. Simply run:
```bash
make
make run OUTPUT_FILE=output.txt NUM_PROCESSES=10
```
- `<OUTPUT_FILE>`: Name of the file where child outputs will be logged.
- `<NUM_PROCESSES>`: Number of child processes (positive integer).

### Alternatively
Compile the program:
```bash
gcc -Wall -o main main.c
```
Execute the program:
```bash
./main <filename> <#processes>
```
- `<filename>`: Name of the file where child outputs will be logged.
- `<#processes>`: Number of child processes (positive integer).

Example:
```bash
./main output.txt 10
```

### Signals
- **SIGINT**: Triggers a graceful shutdown (Ctrl+C).
- **SIGTERM**: Triggers a graceful shutdown.

### Files
- `main.c`: Main program file.
- `Makefile`: Build automation file.

### Cleanup
To remove compiled binaries and temporary files:
```bash
make clean
```

## Author
Exarchou Athos
it2022134@hua.gr
