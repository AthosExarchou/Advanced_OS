//it2022134 Exarchou Athos

/* imports */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>


int main (int argc, char *argv[]) {

    int fd;
    int childs = atoi(argv[2]);
    char *filename = argv[1];

    if ((argc != 3) || (childs <= 0)) {

        printf("On execution:\n-the 1st argument provided, should represent the name of the file to "
               "be created, and\n-the 2nd argument, the (positive) number (>0) of child processes\n");
        return EXIT_FAILURE; //exits with failure
    }
    printf("File name: '%s'\nNumber of child processes: '%d'\n", argv[1], childs);

    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644); //opens/creates, clears file
    //checks for errors
    if (fd == -1) {
        
        perror("Error opening file. Exiting...\n");
        return EXIT_FAILURE; //exits with failure
    }

    sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0644, 1); //initialized to 1
    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        close(fd);
        return EXIT_FAILURE;
    }

    // /* child semaphore for synchronization */
    // sem_t *child_sem = sem_open("/child_semaphore", O_CREAT, 0644, 0);
    // if (child_sem == SEM_FAILED) {
    //     perror("Error creating child semaphore");
    //     close(fd);
    //     sem_close(sem);
    //     sem_unlink("/my_semaphore");
    //     return EXIT_FAILURE;
    // }


    //creates an array of semaphores for child synchronization
    sem_t *child_sems[childs];
    for (int i = 0; i < childs; i++) {
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/child_sem_%d", i);
        child_sems[i] = sem_open(sem_name, O_CREAT, 0644, (i == 0) ? 1 : 0); //first child starts
        if (child_sems[i] == SEM_FAILED) {
            perror("Error creating child semaphore");
            close(fd);
            sem_close(sem);
            sem_unlink("/my_semaphore");
            for (int j = 0; j <= i; j++) {
                sem_unlink(sem_name);
            }
            return EXIT_FAILURE;
        }
    }

    /* parent process message */
    char message[100];
    snprintf(message, sizeof(message), "[PARENT] —> <%d>\n", getpid());
    sem_wait(sem); //acquires semaphore

    if (write(fd, message, strlen(message)) == -1) {
        perror("Error writing to file");
    }
    sem_post(sem); //releases semaphore

    // //parent signals the first child to begin
    // sem_post(child_sem);

    pid_t pid;
    for (int i = 0; i < childs; i++) {

        pid = fork();
        if (pid < 0) {

            perror("Error creating process. Exiting...");
            close(fd); //closes the file before exiting the child process
            sem_close(sem); //closes the semaphore in the child process
            sem_unlink("/my_semaphore"); //removes the semaphore
            // sem_close(child_sem);
            // sem_unlink("/child_semaphore");
            for (int j = 0; j < childs; j++) {
                char sem_name[20];
                snprintf(sem_name, sizeof(sem_name), "/child_sem_%d", j);
                sem_close(child_sems[j]);
                sem_unlink(sem_name);
            }
            return EXIT_FAILURE;
        } else if (pid == 0) {
            
            // if (i > 0) {
            //     sem_wait(child_sem); //waits for the previous child to finish writing
            // }
            sem_wait(child_sems[i]);

            snprintf(message, sizeof(message), "[CHILD] -> <%d>\n", getpid());
            sem_wait(sem); //acquires semaphore
            
            if (write(fd, message, strlen(message)) == -1) {
                perror("Error writing to file");
            }
            sem_post(sem); //releases semaphore

            // if (i < childs - 1) {
            //     sem_post(child_sem); //notifies the next child
            // }
            // 
            // sem_close(sem);

            //signals the next child (if any)
            if (i < childs - 1) {
                sem_post(child_sems[i + 1]);
            }

            //cleans up and exits
            sem_close(sem);
            for (int j = 0; j < childs; j++) {
                sem_close(child_sems[j]);
            }

            close(fd);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < childs; i++) {
        //sem_wait(child_sem); //waits for the previous child to finish
        //waitpid(-1, NULL, 0);
        wait(NULL);
    }

    // sem_close(sem); //closes the semaphore in the parent process
    // close(fd); //closes the file in the parent process

    // sem_unlink("/my_semaphore"); //removes the semaphore
    // sem_unlink("/child_semaphore");

    //cleans up
    sem_close(sem);
    sem_unlink("/my_semaphore");
    for (int i = 0; i < childs; i++) {
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/child_sem_%d", i);
        sem_close(child_sems[i]);
        sem_unlink(sem_name);
    }
    close(fd);

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