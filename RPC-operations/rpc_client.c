/* it2022134 Exarchou Athos */

/* Imports */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Constants for readability */
#define SERVER_PIPE "/tmp/server_pipe"
#define BUFFER_SIZE 512 //client_pipe+request(256+256)=512 -> avoids overflow

int main() {
    char client_pipe[256] = {0}; //initialized with null bytes
    char request[BUFFER_SIZE] = {0}; //initialized with null bytes
    char response[BUFFER_SIZE];

    /* generates a unique pipe name for the client */
    snprintf(client_pipe, sizeof(client_pipe), "/tmp/client_pipe_%d", getpid());
    mkfifo(client_pipe, 0666);

    while (1) { //infinite loop for multiple requests (until "exit" input or termination)
        memset(request, 0, sizeof(request)); //clears request before each input

        printf("Enter request (e.g., 5+3) or 'exit' to quit: ");
        fgets(request, BUFFER_SIZE, stdin);
        request[strcspn(request, "\n")] = 0; //removes the newline character

        /* ensures empty input doesn't send the previous request */
        if (strlen(request) == 0) {  
            printf("No input provided. Please enter a valid request.\n");
            continue; //skips sending request (skips current loop)
        }

        if (strcmp(request, "exit") == 0) {
            break; //exits the loop if the user enters "exit"
        }

        char message[BUFFER_SIZE];
        /* ensures client_pipe and request are truncated if too long */
        snprintf(message, BUFFER_SIZE, "%.*s %.*s", 100, client_pipe, 100, request);

        /* sends request to the server */
        int server_fd = open(SERVER_PIPE, O_WRONLY);
        if (server_fd == -1) {
            perror("Error opening server pipe");
            unlink(client_pipe);
            exit(EXIT_FAILURE);
        }

        write(server_fd, message, strlen(message) + 1);
        close(server_fd);

        /* waits for the response from the server */
        int client_fd = open(client_pipe, O_RDONLY);
        if (client_fd == -1) {
            perror("Error opening client pipe");
            unlink(client_pipe);
            exit(EXIT_FAILURE);
        }

        memset(response, 0, sizeof(response)); //clears response before reading
        read(client_fd, response, BUFFER_SIZE);
        printf("Server response: %s\n", response);

        close(client_fd);
    }

    /* cleanup */
    unlink(client_pipe);

    return 0;
}
