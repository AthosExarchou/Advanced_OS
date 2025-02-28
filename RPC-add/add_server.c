#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "add.h"

/* Constants for readability */
#define READ 0
#define WRITE 1
#define NUM_WORKERS 4

int to_child[NUM_WORKERS][2]; //pipes for sending tasks to workers
int to_parent[NUM_WORKERS][2]; //pipes for receiving results from workers
pid_t workers[NUM_WORKERS]; //worker process IDs
sem_t *worker_sems[NUM_WORKERS]; //worker availability semaphores

void init_workers() {
    for (int i = 0; i < NUM_WORKERS; i++) {
        pipe(to_child[i]);  
        pipe(to_parent[i]);  

        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/worker_sem_%d", i);
        sem_unlink(sem_name);
        worker_sems[i] = sem_open(sem_name, O_CREAT, 0644, 1); //worker is available initially

        if (worker_sems[i] == SEM_FAILED) {
            perror("Error creating worker semaphore");
            exit(1);
        }

        workers[i] = fork();
        if (workers[i] == 0) { //worker process
            close(to_child[i][WRITE]); //closes the writing end of the parent pipe
            close(to_parent[i][READ]); //closes the reading end of the child pipe

            while(1) {
                int numbers[5];

                read(to_child[i][READ], numbers, sizeof(numbers)); //reads 5 numbers
                printf("Processing task: %d + %d + %d + %d + %d\n",
                    numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]); //displays said numbers

                int result = numbers[0] + numbers[1] + numbers[2] + numbers[3] + numbers[4];

                write(to_parent[i][WRITE], &result, sizeof(int)); //sends result to the parent
                
                sem_post(worker_sems[i]); //releases worker
            }
            exit(0);
        } else {
            close(to_child[i][READ]); //closes the reading end of the parent pipe
            close(to_parent[i][WRITE]); //closes the writing end of the child pipe
        }
    }
}

void cleanup() {
    for (int i = 0; i < NUM_WORKERS; i++) {
        close(to_child[i][WRITE]);
        close(to_parent[i][READ]);
        kill(workers[i], SIGTERM);
        waitpid(workers[i], NULL, 0); //waits for each worker to finish
        sem_close(worker_sems[i]);
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/worker_sem_%d", i);
        sem_unlink(sem_name);
    }
}

void signalHandler(int sig) {
    cleanup();
    exit(0);
}

int * add_1_svc(numbers *nums, struct svc_req *req) {

    static int result;

    printf("Received RPC request: %d, %d, %d, %d, %d\n",
        nums->a, nums->b, nums->c, nums->d, nums->e);

    int assigned_worker = -1;

    for (int i = 0; i < NUM_WORKERS; i++) {
        int sem_value;
        sem_getvalue(worker_sems[i], &sem_value);

        if (sem_value > 0) { //acquires a worker only if it's available
            if (sem_trywait(worker_sems[i]) == 0) {
                assigned_worker = i;
                break;
            }
        }
    }

    if (assigned_worker == -1) {
        printf("No workers currently available!\n");
        result = -1;
        return &result;
    }

    int numbers[5] = {nums->a, nums->b, nums->c, nums->d, nums->e};
    write(to_child[assigned_worker][WRITE], numbers, sizeof(numbers));

    if (read(to_parent[assigned_worker][READ], &result, sizeof(int)) <= 0) {
        printf("Error: Worker %d did not respond\n", assigned_worker);
        result = -1;
    } //reads result from worker

    return &result;
}
