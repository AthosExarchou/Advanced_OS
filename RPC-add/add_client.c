#include <stdio.h>
#include <stdlib.h>
#include "add.h"

void add_prog_1(char *host, int num1, int num2, int num3, int num4, int num5) {
    
    CLIENT *clnt;
    int *result;
    numbers nums;

    /* Create RPC client */
    clnt = clnt_create(host, ADD_PROG, ADD_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    /* Assign values to the struct */
    nums.a=num1;
    nums.b=num2;
    nums.c=num3;
    nums.d=num4;
    nums.e=num5;

    /* Make RPC */
    result = add_1(&nums, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "RPC failed");
    } else {
        printf("Result: %d\n", *result);
    }

    clnt_destroy(clnt); //cleanup client
}

int main(int argc, char *argv[]) {

    if (argc < 7) { //expecting 5 numbers to add
        printf("Usage: %s <server_host> num1 num2 num3 num4 num5\n", argv[0]);
        exit(1);
    }

    char *host = argv[1];
    printf("Connecting to RPC server at %s...\n", host);

    add_prog_1(host, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    return 0;
}
