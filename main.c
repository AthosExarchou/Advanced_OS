/* it2022134 Exarchou Athos */

/* Imports */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

/* Constants for readability */
#define READ 0
#define WRITE 1

/* Structs */
typedef enum {
    GENERAL_WORKER, //handles standard tasks
    IO_WORKER, //handles IO operations
    COMPUTATION_WORKER, //handles CPU-heavy tasks
    MIXED_WORKER //can do both IO & computation
} WorkerType;

typedef enum {
    TASK_GENERAL,
    TASK_IO,
    TASK_COMPUTATION
} TaskType;

typedef struct {
    int id;
    TaskType type;
    char data[100]; //task details
} Task;

typedef struct {
    int to_child[2]; //pipe from parent to child
    int to_parent[2]; //pipe from child to parent
    pid_t child_pid; //child PID
    char child_name[20]; //child name
    int is_idle; //indicates whether the child is idle
    WorkerType type; //type of worker
} ChildInfo;

/* Global variables */
ChildInfo *children = NULL; //child information
sem_t *sem = NULL; //semaphore
sem_t **child_sems = NULL; //array of semaphores for child synchronization
int num_children = 0; //number of children
int fd = -1; //file descriptor

/* Functions */

/* resource cleanup function */
void cleanup_resources(ChildInfo *children, int num_children, sem_t *sem, int fd, sem_t **child_sems) {
    for (int i = 0; i < num_children; i++) {
        close(children[i].to_child[READ]);
        close(children[i].to_child[WRITE]);
        close(children[i].to_parent[READ]);
        close(children[i].to_parent[WRITE]);
    }
    free(children); //frees the allocated memory for children
    sem_close(sem); //closes the semaphore
    sem_unlink("/my_semaphore"); //unlinks the semaphore
    close(fd); //closes the file descriptor
    for (int i = 0; i < num_children; i++) {
        if (child_sems[i]) {
            sem_close(child_sems[i]);
            char sem_name[20];
            snprintf(sem_name, sizeof(sem_name), "/child_sem_%d", i);
            sem_unlink(sem_name);
        }
    }
}

/* signal handler function for termination signals */
void signalHandler(int sig) {
    printf("Parent received signal <%d>. Shutting down...\n", sig);

    /* when a signal is received, sends "exit" to all children */
    for (int i = 0; i < num_children; i++) {
        if (write(children[i].to_child[WRITE], "exit", strlen("exit")) == -1) {
            perror("Error writing exit to pipe");
        }
    }

    /* waits for all children to finish their tasks */
    for (int i = 0; i < num_children; i++) {
        waitpid(children[i].child_pid, NULL, 0);
    }

    /* cleanup */
    cleanup_resources(children, num_children, sem, fd, child_sems);
    printf("All children have been terminated.\n");
    exit(EXIT_SUCCESS);
}

/* task distributor */
void distribute_task(int child_index, Task task) {
        if (children[child_index].is_idle) {
            /* sends task data to the child process */
            if (write(children[child_index].to_child[WRITE], task.data, strlen(task.data)) == -1) {
                perror("Error writing task to pipe");
                return;
            }
            sem_post(child_sems[child_index]); //signals the child process

            children[child_index].is_idle = 0; //child becomes busy

            printf("Assigned %s to %s (PID: %d)\n", task.data, children[child_index].child_name, children[child_index].child_pid);
            return;
        }
    printf("Worker not available for task: %s\n", task.data);
}

/* Main program */
int main (int argc, char *argv[]) {

    char *filename = argv[1];
    int num_children = atoi(argv[2]);

    if ((argc != 3) || (num_children <= 0)) {

        printf("On execution:\n-the 1st argument provided, should represent the name of the file to "
               "be created, and\n-the 2nd argument, the (positive) number (>0) of child processes\n");
        return EXIT_FAILURE; //exits with failure
    }
    printf("File name: '%s'\nNumber of child processes: '%d'\n", filename, num_children);

    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644); //opens/creates, clears file
    //checks for errors
    if (fd == -1) {
        perror("Error opening file. Exiting...\n");
        return EXIT_FAILURE; //exits with failure
    }

    /* semaphore initialization */
    sem = sem_open("/my_semaphore", O_CREAT, 0644, 1); //initialized to 1
    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        cleanup_resources(NULL, 0, NULL, fd, NULL);
        return EXIT_FAILURE;
    }

    /* allocates memory for children */
    children = malloc(sizeof(ChildInfo) * num_children);
    if (!children) {
        perror("Memory allocation failed for children");
        cleanup_resources(NULL, 0, sem, fd, NULL);
        return EXIT_FAILURE;
    }

    child_sems = malloc(num_children * sizeof(sem_t *));
    if (!child_sems) {
        perror("Memory allocation failed for child semaphores");
        cleanup_resources(children, 0, sem, fd, NULL);
        return EXIT_FAILURE;
    }

    /* creates an array of semaphores for child synchronization */
    for (int i = 0; i < num_children; i++) {
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/child_sem_%d", i);
        /* unlinks any existing semaphores with the same name */
        sem_unlink(sem_name);
        child_sems[i] = sem_open(sem_name, O_CREAT, 0644, (i == 0) ? 1 : 0); //first child starts
        if (child_sems[i] == SEM_FAILED) {
            perror("Error creating child semaphore");
            cleanup_resources(NULL, i, sem, fd, child_sems);
            return EXIT_FAILURE;
        }
    }

    /* pipe creation */
    for (int i = 0; i < num_children; i++) {
        if (pipe(children[i].to_child) == -1 || pipe(children[i].to_parent) == -1) {
            perror("Pipe creation failed");
            cleanup_resources(children, i, sem, fd, child_sems);
            return EXIT_FAILURE;
        }

        /* stores each child's name into the corresponding child */
        snprintf(children[i].child_name, sizeof(children[i].child_name), "Child %d", i + 1);

        children[i].is_idle = 1; //all children are idle at the start

        /* assigns a worker type to each child (round-robin) */
        children[i].type = (i % 4 == 0) ? COMPUTATION_WORKER :
                           (i % 4 == 1) ? IO_WORKER :
                           (i % 4 == 2) ? MIXED_WORKER :
                                          GENERAL_WORKER;

        pid_t pid = fork(); //new child process
        if (pid < 0) {
            perror("Fork failed");
            cleanup_resources(children, i, sem, fd, child_sems);
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            /* child process */
            close(children[i].to_child[WRITE]); //closes the writing end of the parent pipe
            close(children[i].to_parent[READ]); //closes the reading end of the child pipe

            while (1) {
                sem_wait(child_sems[i]);
                char buffer[100];
                ssize_t bytes_read = read(children[i].to_child[READ], buffer, sizeof(buffer) - 1);
                
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    if (strcmp(buffer, "exit") == 0) {
                        break;
                    }

                    printf("<CHILD %d> Received message: %s\n", i + 1, buffer);

                    srand(time(NULL)); //randomization
                    int timer;
                    /* children sleep depending on their role */
                    switch (children[i].type) {
                        case GENERAL_WORKER:
                            timer = rand() % 3 + 1; //1-3 sec
                            break;
                        case IO_WORKER:
                            timer = rand() % 6 + 2; //2-7 sec
                            break;
                        case COMPUTATION_WORKER:
                            timer = rand() % 5 + 3; //3-7 sec
                            break;
                        case MIXED_WORKER:
                            timer = rand() % 4 + 2; //2-5 sec
                            break;
                        default:
                            timer = 2; //default timer for invalid
                    }

                    printf("<CHILD %d> Sleeping for %d seconds...\n", i + 1, timer);
                    sleep(timer);

                    /* child process writes to file */
                    char message[100];
                    snprintf(message, sizeof(message), "<%d> -> <%s>\n", getpid(), children[i].child_name);
                    if (write(fd, message, strlen(message)) == -1) {
                        perror("Error writing to file");
                    }

                    /* notifies parent process */
                    if (write(children[i].to_parent[WRITE], "done", strlen("done")) == -1) {
                        perror("Error sending acknowledgment to parent");
                    }
                } else {
                    perror("Error reading from pipe");
                }
            }

            close(children[i].to_child[READ]); //closes the reading end of the parent pipe
            close(children[i].to_parent[WRITE]); //closes the writing end of the child pipe
            sem_close(sem);
            close(fd);
            exit(EXIT_SUCCESS);
        } else {
            /* parent process */
            children[i].child_pid = pid;
            close(children[i].to_child[READ]);
            close(children[i].to_parent[WRITE]);
        }
    }
    
    /* sets up signal handler for graceful shutdown */
    signal(SIGINT, signalHandler); //handles Ctrl-C
    signal(SIGTERM, signalHandler); //handles termination signals

    int task_counter = 0;
    while (task_counter < num_children * 2) {
        for (int i = 0; i < num_children; i++) {
            if (children[i].is_idle) {

                Task new_task;
                new_task.id = task_counter + 1;
                snprintf(new_task.data, sizeof(new_task.data), "Task %d", new_task.id);

                if (new_task.id % 3 == 0) {
                    new_task.type = TASK_COMPUTATION;
                } else if (new_task.id % 2 == 0) {
                    new_task.type = TASK_IO;
                } else {
                    new_task.type = TASK_GENERAL;
                }

                /* task distribution */
                distribute_task(i, new_task);

                char buffer[10];
                ssize_t bytes_read = read(children[i].to_parent[READ], buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    if (strcmp(buffer, "done") == 0) {
                        printf("%s completed: %s\n", children[i].child_name, new_task.data);
                        children[i].is_idle = 1;
                    }
                } else {
                    perror("Error reading acknowledgment");
                }
                task_counter++;
            }
        }
    }
    
    /* sends exit message to children */
    for (int i = 0; i < num_children; i++) {
        if (write(children[i].to_child[WRITE], "exit", strlen("exit")) == -1) {
            perror("Error writing exit to pipe");
        }
        sem_post(child_sems[i]); //notifies child to terminate
    }

    /* parent waits for children to finish their tasks */
    for (int i = 0; i < num_children; i++) {
        waitpid(children[i].child_pid, NULL, 0);
    }

    /* cleanup */
    cleanup_resources(children, num_children, sem, fd, child_sems);
    printf("Tasks completed.\n");
    return EXIT_SUCCESS;
}
