#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

void collatz(int);
int isPositiveInt(char *);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No arguments supplied.");
        return 0;
    }
    if (!isPositiveInt(argv[1])) {
        printf("Argument supplied is not a positive integer.");
        return 0;
    }

    int start = atoi(argv[1]);

    if (fork() == 0) {
        collatz(start);
    }
    int status;
    wait(&status);

    return status;
}

void collatz(int start) {
    while (start > 1) {
        printf("%d\n", start);
        if (start % 2 == 0) {
            start /= 2;
        }
        else {
            start = start * 3 + 1;
        }
    }
    printf("%d\n", start);
}

int isPositiveInt(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}
