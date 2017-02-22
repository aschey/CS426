#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int avg;
int min;
int max;
struct argStruct {
    int length;
    int *vals;
};
int *createIntArray(int, int, char *[]);
void *getAvg(void *);
void *getMin(void *);
void *getMax(void *);

int main(int argc, char *argv[]) {
    struct argStruct arguments;
    arguments.length = argc - 1;
    arguments.vals = createIntArray(1, argc, argv);
    pthread_t avgThread;
    pthread_t minThread;
    pthread_t maxThread;
    pthread_create(&avgThread, NULL, &getAvg, &arguments);
    pthread_create(&minThread, NULL, &getMin, &arguments);
    pthread_create(&maxThread, NULL, &getMax, &arguments);
    pthread_join(avgThread, NULL);
    pthread_join(minThread, NULL);
    pthread_join(maxThread, NULL);
    printf("The average value is %d\n", avg);
    printf("The minimum value is %d\n", min);
    printf("The maximum value is %d\n", max);
    return 0;
}

int *createIntArray(int offset, int length, char *nums[]) {
    int *result = malloc((length - offset) * sizeof(int));
    for (int i = offset; i < length; i++) {
        result[i - offset] = atoi(nums[i]);
    }
    return result;
}

void *getAvg(void *args) {
    printf("Entering getAvg\n");
    struct argStruct *arguments = args;
    int sum = 0;
    for (int i = 0; i < arguments->length; i++) {
        sum += arguments->vals[i];
    }
    avg = sum / arguments->length;

    printf("Exiting getAvg\n");
    return NULL;
}

void *getMin(void *args) {
    printf("Entering getMin\n");
    struct argStruct *arguments = args;
    min = arguments->vals[0];
    for (int i = 1; i < arguments->length; i++) {
        if (arguments->vals[i] < min) {
            min = arguments->vals[i];
        }
    }

    printf("Exiting getMin\n");
    return NULL;
}

void *getMax(void *args) {
    printf("Entering getMax\n");
    struct argStruct *arguments = args;
    max = arguments->vals[0];
    for (int i = 1; i < arguments->length; i++) {
        if (arguments->vals[i] > max) {
            max = arguments->vals[i];
        }
    }

    printf("Exiting getMax\n");
    return NULL;
}
