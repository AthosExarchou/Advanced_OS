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
- Support for different worker types to handle heterogeneous workloads.

## Workloads and Worker Profiles

The program supports different worker profiles to handle various workloads efficiently. Each worker type is specialized to process specific kinds of tasks:

### Worker Types

1. General Worker:
Handles standard tasks that do not require specialized processing.

2. I/O Worker:
Focused on handling I/O-bound tasks such as reading/writing files or network communication.

3. Computation Worker:
Handles CPU-intensive tasks such as mathematical calculations and data processing.

4. Mixed Worker:
Capable of handling both I/O-bound and computation-heavy tasks, providing flexibility in workload distribution.

### Task Types

Each task is categorized based on its workload type:

1. General Task:
A standard task that does not require specific handling.

2. I/O Task:
Involves file operations, database queries, or other I/O-intensive operations.

3. Computation Task:
Requires CPU-intensive processing, such as simulations or numerical computations.

Tasks are assigned to workers based on their specialization, ensuring efficient resource utilization and optimized execution times.

## How to run

### Prerequisites
Ensure you have installed in your system the following:
- GCC compiler (for compiling C code).
- Linux environment (for pipe and semaphore usage).
- `make` (for building the project).

### Compile and execute
To compile and run the program, use the provided Makefile. Simply run:
```bash
make
make run OUTPUT_FILE=output.txt NUM_PROCESSES=5
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
./main output.txt 5
```

### Signals
- **SIGINT**: Triggers a graceful shutdown (Ctrl+C).
- **SIGTERM**: Triggers a graceful shutdown.

### Files
- `main.c`: Main program file.
- `Makefile`: Build automation file.

### Cleanup
To remove compiled binaries:
```bash
make clean
```

## Author
Exarchou Athos
it2022134@hua.gr
