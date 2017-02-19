#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_LINE 80
#define HIST_SIZE 10

char *removeN(char *, int);
int parseArgs(char *[], char *);
int parseHistory(char *[], char *);
int isPositiveInt(char *);

typedef struct {
    size_t used;
    size_t capacity;
    char **store;
} Array;

void initArray(Array *);
void add(char *val, Array *array);
void dispose(Array *array);
char *get(int index, Array array);

int main() {
    int currentCommand = 0;
    int status = 0;
    char *line = NULL;
    char *args[MAX_LINE / 2 + 1];
    size_t size;
    Array history;

    initArray(&history);
    while (1) {
        printf("osh> ");
        fflush(stdout);
        if (getline(&line, &size, stdin) != -1) {
            int arraySize;
            if (strcmp("!!\n", line) == 0) {
                // Get the most recent command
                line = get(currentCommand - 1, history);
                // Load the command into "args"
                arraySize = parseHistory(args, line);
                // Just return to the prompt if no commands have been executed yet.
                if (arraySize == 0) {
                    continue;
                }
            }
            else if (line[0] == '!') {
                char *commandNumber = removeN(line, 1);
                if (!isPositiveInt(commandNumber)) {
                    printf("Argument must be a positive integer.\n");
                    continue;
                }
                int chosenCommand = atoi(commandNumber) - 1;
                // Get the chosen command
                line = get(chosenCommand, history);
                // Load the command into "args"
                arraySize = parseHistory(args, line);
                if (arraySize == 0) {
                    printf("No such command in history\n");
                    continue;
                }
                free(commandNumber);
            }
            else if (strcmp("history\n", line) == 0) {
                char *val = NULL;
                // Get the top 10 commands, or just the top n if n < 10
                for (int i = currentCommand; i > currentCommand - 10 && i > 0; i--) {
                    int currentIndex = i - 1;
                    val = strdup(get(currentIndex, history));
                    // If we reach a NULL, there are fewer than 10 commands
                    if (val == NULL) {
                        continue;
                    }
                    printf("%d %s", currentIndex + 1, val);
                }
                free(val);
                continue;
            }
            else if (strcmp("exit\n", line) == 0) {
                status = 0;
                break;
            }
            else {
                // Split the string and store it in "args"
                arraySize = parseArgs(args, line);
            }

            int waitForChild = 1;
            if (strcmp("&", args[arraySize - 1]) == 0) {
                // Remove the "&" so it isn't passed to execvp
                args[arraySize - 1] = NULL;
                waitForChild = 0;
            }

            // Save the command in the history array
            add(line, &history);
            currentCommand++;

            if (fork() == 0) {
                status = execvp(args[0], args);
                // Status of -1 indicates error
                if (status == -1) {
                    perror("error");
                    break;
                }
            }
            else if (waitForChild == 1) {
                wait(&status);
                // Break out of the process didn't exit correctly
                if (!WIFEXITED(status)) {
                    break;
                }
            }
            // Wait for child process to finish printing
            sleep(1);
        }
    }
    free(line);
    dispose(&history);
    // Return the exit status of the command
    return status;
}

char *removeN(char *c, int n) {
    // Remove the first n characters from the string
    size_t newStrLen = strlen(c) + 1 - n;
    char *newString = malloc(newStrLen * sizeof(char));
    return strncpy(newString, &c[n], newStrLen);
}

int isPositiveInt(char *str) {
    // Don't check newline character
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

int parseArgs(char *args[], char *line) {
    // Split the line on each space character
    char *savePtr;
    char *lineCopy = strdup(line);
    char *token = strtok_r(lineCopy, " ", &savePtr);
    int arraySize = 0;

    do {
        size_t last = strlen(token) - 1;
        // Remove newlines because execvp doesn't like them
        if (token[last] == '\n') {
            token[last] = '\0';
        }
        // Add each token to the args array
        args[arraySize++] = strdup(token);
    }
    while ((token = strtok_r(NULL, " ", &savePtr)));

    args[arraySize] = NULL;

    free(lineCopy);

    return arraySize;
}

int parseHistory(char *args[], char *line) {
    if (line == NULL) {
        return 0;
    }
    printf("%s\n", line);
    return parseArgs(args, line);
}

void initArray(Array *array) {
    array->store = malloc(HIST_SIZE * sizeof(char *));
    array->used = 0;
    array->capacity = HIST_SIZE;
}

void add(char *val, Array *array) {
    // Multiply the capacity by two when the array gets full
    if (array->used == array->capacity) {
        array->capacity *= 2;
        array->store = (char **)realloc(array->store, array->capacity * sizeof(char *));
    }
    array->store[array->used++] = strdup(val);
}

void dispose(Array *array) {
    free(array->store);
}

char *get(int index, Array array) {
    if (index >= array.used || index < 0) {
        return NULL;
    }
    return strdup(array.store[index]);
}
