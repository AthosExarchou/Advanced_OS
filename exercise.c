/* it2022134 Exarchou Athos */

/* imports */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

/* constants for readability */
#define READ 0
#define WRITE 1

typedef struct {
    int to_child[2]; //pipe from parent to child
    int to_parent[2]; //pipe from child to parent
    pid_t child_pid; //child PID
    char child_name[20]; //child name
} ChildInfo;

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

int main (int argc, char *argv[]) {

    char *filename = argv[1];
    int num_children = atoi(argv[2]);

    if ((argc != 3) || (num_children <= 0)) {

        printf("On execution:\n-the 1st argument provided, should represent the name of the file to "
               "be created, and\n-the 2nd argument, the (positive) number (>0) of child processes\n");
        return EXIT_FAILURE; //exits with failure
    }
    printf("File name: '%s'\nNumber of child processes: '%d'\n", filename, num_children);

    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644); //opens/creates, clears file
    //checks for errors
    if (fd == -1) {
        perror("Error opening file. Exiting...\n");
        return EXIT_FAILURE; //exits with failure
    }

    /* semaphore initialization */
    sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0644, 1); //initialized to 1
    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        cleanup_resources(NULL, 0, NULL, fd, NULL);
        return EXIT_FAILURE;
    }

    /* creates an array of semaphores for child synchronization */
    sem_t *child_sems[num_children];
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

    /* allocates memory for children */
    ChildInfo *children = malloc(sizeof(ChildInfo) * num_children);
    if (!children) {
        perror("Memory allocation failed");
        cleanup_resources(NULL, 0, sem, fd, child_sems);
        return EXIT_FAILURE;
    }
    
    /* pipe creation and fork */
    for (int i = 0; i < num_children; i++) {
        if (pipe(children[i].to_child) == -1 || pipe(children[i].to_parent) == -1) {
            perror("Pipe creation failed");
            cleanup_resources(children, i, sem, fd, child_sems);
            return EXIT_FAILURE;
        }

        snprintf(children[i].child_name, sizeof(children[i].child_name), "Child %d", i + 1);

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            cleanup_resources(children, i, sem, fd, child_sems);
            return EXIT_FAILURE;
        } else if (pid == 0) {
            /* child process */
            close(children[i].to_child[WRITE]);
            close(children[i].to_parent[READ]);

            sem_wait(child_sems[i]);

            char buffer[100];
            ssize_t bytes_read = read(children[i].to_child[READ], buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("<CHILD %d> Received message: %s\n", i + 1, buffer);
                sleep(1);

                /* child processes write to file */
                char message[100];
                snprintf(message, sizeof(message), "<%d> -> <%s>\n", getpid(), children[i].child_name);
                if (write(fd, message, strlen(message)) == -1) {
                    perror("Error writing to file");
                }
                if (i < num_children - 1) {
                    sem_post(child_sems[i + 1]);
                }
                /* notifies parent */
                if (write(children[i].to_parent[WRITE], "done", strlen("done")) == -1) {
                    perror("Error sending acknowledgment to parent");
                }
            } else {
                perror("Error reading from pipe");
            }

            close(children[i].to_child[READ]);
            close(children[i].to_parent[WRITE]);
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

    /* parent sends messages to children */
    for (int i = 0; i < num_children; i++) {
        char message[100];
        snprintf(message, sizeof(message), "Hello child, I am your father and I call you: %s.", children[i].child_name);
        
        if (write(children[i].to_child[WRITE], message, strlen(message)) == -1) {
            perror("Error writing to pipe");
            cleanup_resources(children, i + 1, sem, fd, child_sems);
            return EXIT_FAILURE;
        }
        sem_post(child_sems[i]); //lets the current child proceed
        sem_wait(child_sems[i]); //waits for current child to finish
        close(children[i].to_child[WRITE]);
    }

    /* parent waits for children to answer */
    for (int i = 0; i < num_children; i++) {
        char buffer[10];
        ssize_t bytes_read = read(children[i].to_parent[READ], buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "done") == 0) {
                printf("%s has completed its task.\n", children[i].child_name);
            }
        } else {
            perror("Error reading acknowledgment from child");
            cleanup_resources(children, num_children, sem, fd, child_sems); //cleans up if read fails
            return EXIT_FAILURE;
        }
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

/*
** Compile the program:
**    gcc -Wall -o exc exercise.c
**
** Execute the program:
**    ./exc <file_name> <#processes>
**
** Where:
**    <file_name>  : Name of the file to be created or opened.
**    <#processes> : Positive integer representing the number of child processes.
*/
