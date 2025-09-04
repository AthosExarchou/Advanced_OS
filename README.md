# RPC Project

## Project Overview
This project is an exercise for the Advanced Operating Systems course at Harokopio
University of Athens, Dept. of Informatics and Telematics. It implements a worker pool paradigm using processes and pipes for inter-process communication. The parent process distributes tasks to child processes, which execute them and respond with results. The program ensures proper synchronization using semaphores and supports a graceful shutdown on receiving termination signals (e.g., SIGINT or SIGTERM). Additionally, the project incorporates Remote Procedure Call (RPC) functionality to facilitate client-server communication, allowing remote execution of operations (currently, only the addition of 5 numbers is supported).

### Features
- Task distribution to child processes.
- Bidirectional communication using pipes.
- Semaphore-based synchronization.
- Graceful shutdown with resource cleanup.
- Dynamic handling of tasks and child processes.
- Support for different worker types to handle heterogeneous workloads.
- Remote Procedure Call (RPC) support for executing operations remotely.

## RPC Implementation

### Overview

The RPC mechanism enables a client to send computation requests to the server, which delegates the tasks to worker processes. The results are then sent back to the client.

### Components

- RPC Server: Listens for client requests and distributes tasks to workers.
- RPC Client: Sends computation requests to the server and receives responses.
- Worker Pool: Processes the assigned tasks and returns results.
- Inter-Process Communication (IPC): Pipes are used for worker-server communication, while semaphores handle synchronization.

### Supported RPC Operations

Addition Operation: Clients can send a set of numbers (5) to the server for summation.

Worker-based Parallel Processing: The server assigns computations to available workers, ensuring efficiency.

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
- `rpcbind` (to manage RPC calls).

### Compile and execute
To compile and run the program, use the provided Makefile. Simply run:
```bash
make
make run OUTPUT_FILE=output.txt NUM_PROCESSES=5
```
- `<OUTPUT_FILE>`: Name of the file where child outputs will be logged.
- `<NUM_PROCESSES>`: Number of child processes (positive integer).

Note that, for these commands to work, you need to be in the `/Advanced_OS` directory.

### Compiling & Running the RPC Server
To compile the RPC server, run:
```bash
gcc -o add_server add_svc.c add_server.c add_xdr.c -I/usr/include/tirpc -ltirpc
```
To start the RPC server, run:
```bash
./add_server
```
Note that, for the server commands to work, you need to be in the `/Advanced_OS/RPC-add` directory.

### Compiling & Running the RPC Client
To compile the RPC client, run:
```bash
gcc -o add_client add_clnt.c add_client.c add_xdr.c -I/usr/include/tirpc -ltirpc
```
To send requests to the RPC server, run:
```bash
./add_client <server_host> num1 num2 num3 num4 num5
```
The client will attempt to connect to the server and send a task request. The server will process the request and return the result.

Explementary client execution:
```bash
./add_client localhost 10 20 30 40 50
```

Note that, for the client commands to work, you need to be in the `/Advanced_OS/RPC-add` directory.

### Run RPC with Docker
Build the image:
```bash
docker build -t rpc_server .
```
Run the container:
```bash
docker run -it --name rpc_server rpc_server
```
In case the container already exists, remove it:
```bash
docker rm -f rpc_server
```
Execute the command:
```bash
make -f Makefile.add
```
Get the container’s IP address with:
```bash
docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' rpc_server
```
Run the client (with the ip that was displayed):
```bash
./add_client <container_ip_address> num1 num2 num3 num4 num5
```

### Alternatively (non RPC)
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

Explementary execution:
```bash
./main output.txt 5
```

### Signals
- **SIGINT**: Triggers a graceful shutdown (Ctrl+C).
- **SIGTERM**: Triggers a graceful shutdown.

### Files
- `main.c`: Entry point for the worker-based task distribution system
- `Makefile`: Build automation file to compile the project.
- `add_server.c`: Server-side implementation for the RPC system.
- `add_client.c`: Client-side implementation for making RPC requests.
- `add_svc.c`: Generated RPC service skeleton for the server.
- `add_xdr.c`: XDR (External Data Representation) serialization/deserialization logic.
- `README.md`: Documentation and instructions for running the project.
- and more...

### Cleanup
To remove compiled binaries:
- For the worker-based task distribution system, run:
```bash
make clean
```
- For the RPC client-server system, run:
```bash
make -f Makefile.add clean
```

## Author

- **Name**: Exarchou Athos
- **Student ID**: it2022134
- **Email**: it2022134@hua.gr, athosexarhou@gmail.com

## License
This project is licensed under the MIT License.
