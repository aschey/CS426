#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void collatz(int);

int main(int argc, char *argv[]) {
    int start = atoi(argv[1]);
    int status;

    if (fork() == 0) {
        collatz(start);
    }
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