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
#define BUFFER_SIZE 256

void process_request(const char *request, char *response) {
    int num1, num2, result;
    char operation;
    
    if (sscanf(request, "%d %c %d", &num1, &operation, &num2) == 3) {
        switch (operation) {
            case '+': result = num1 + num2; break;
            case '-': result = num1 - num2; break;
            case '*': result = num1 * num2; break;
            case '/': result = (num2 != 0) ? num1 / num2 : 0; break;
            default:
                snprintf(response, BUFFER_SIZE, "Invalid operation");
                return;
        }
        snprintf(response, BUFFER_SIZE, "Result: %d", result);
    } else {
        snprintf(response, BUFFER_SIZE, "Invalid request format");
    }
}

int main() {
    char buffer[BUFFER_SIZE];
    
    /* creates the server pipe */
    mkfifo(SERVER_PIPE, 0666);
    
    printf("RPC Server started. Waiting for client requests...\n");

    while (1) {
        int server_fd = open(SERVER_PIPE, O_RDONLY);
        if (server_fd == -1) {
            perror("Error opening server pipe");
            exit(EXIT_FAILURE);
        }

        /* reads client request */
        if (read(server_fd, buffer, BUFFER_SIZE) > 0) {
            char client_pipe[50];
            char request[BUFFER_SIZE], response[BUFFER_SIZE];

            sscanf(buffer, "%s %[^\n]", client_pipe, request); //reads until a newline
            printf("Received request from %s: %s\n", client_pipe, request);

            process_request(request, response);

            /* sends response to the client */
            int client_fd = open(client_pipe, O_WRONLY);
            if (client_fd != -1) {
                write(client_fd, response, strlen(response) + 1);
                close(client_fd);
            } else {
                perror("Error opening client pipe");
            }
        }

        close(server_fd);
    }

    /* unlinks the server pipe */
    unlink(SERVER_PIPE);
    return 0;
}
