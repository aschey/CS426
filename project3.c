#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <semaphore.h>

typedef int buffer_item;

#define BUFFER_SIZE 5
#define RANDRANGE(min, max) ((rand() - min) % (max - min + 1) + min)

// Starting index of circular queue. Where to insert values.
int in;
// Ending index of circular queue. Where to pull values from.
int out;
// How many values are currently in the buffer.
int size;
buffer_item buffer[BUFFER_SIZE];

void init_buffer();
int insert_item(buffer_item);
int remove_item(buffer_item *);

void *produce(void *);
void *consume(void *);

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

int main(int argc, char *argv[]) {
    unsigned int sleep_time = (unsigned)atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);

    init_buffer();

    // Dummy tid. Don't actually need to use this since pthread_join isn't used.
    pthread_t tid;
    for (int i = 0; i < num_producers; i++) {
        pthread_create(&tid, NULL, produce, NULL);
    }
    for (int i = 0; i < num_consumers; i++) {
        pthread_create(&tid, NULL, consume, NULL);
    }
    sleep(sleep_time);
    return 0;
}

void *produce(void *params) {
    long int tid = syscall(SYS_gettid);
    while(1) {
        unsigned int sleep_time = (unsigned)RANDRANGE(0, 3);
        printf("(Producer) Thread %ld sleeping %d seconds\n", tid, sleep_time);
        sleep(sleep_time);
        buffer_item item = rand();
        printf("(Producer) Thread %ld produced item %d\n", tid, item);
        // Wait for at least one spot in the buffer to open
        sem_wait(&empty);
        // Don't modify buffer during insertion
        pthread_mutex_lock(&mutex);
        if(insert_item(item)) {
            printf("(Producer) Thread %ld can't insert item %d; buffer full\n", tid, item);
        }
        else {
            printf("(Producer) Thread %ld inserted item %d\n", tid, item);
            printf("(Producer) Buffer size after insert is %d\n", size);
        }
        pthread_mutex_unlock(&mutex);
        // Update the amount of items in the buffer
        sem_post(&full);
    }
}

void *consume(void *params) {
    long int tid = syscall(SYS_gettid);
    while(1) {
        unsigned int sleep_time = (unsigned)RANDRANGE(0, 3);
        printf("(Consumer) Thread %ld sleeping %d seconds\n", tid, sleep_time);
        sleep(sleep_time);
        // Wait for at least one item to appear in the buffer
        sem_wait(&full);
        // Don't allow modification of the buffer during consumption
        pthread_mutex_lock(&mutex);
        buffer_item item;
        if (remove_item(&item)) {
            printf("(Consumer) Thread %ld can't remove any items; buffer full\n", tid);
        }
        else {
            printf("(Consumer) Thread %ld removed item %d\n", tid, item);
            printf("(Consumer) Buffer size after removal is %d\n", size);
        }
        pthread_mutex_unlock(&mutex);
        // Update the buffer size
        sem_post(&empty);
        printf("(Consumer) Thread %ld consumed item %d\n", tid, item);
    }
}

void init_buffer() {
    in = 0;
    out = 0;
    size = 0;
    pthread_mutex_init(&mutex, NULL);
    // Initially BUFFER_SIZE empty slots
    sem_init(&empty, 0, BUFFER_SIZE);
    // Initially zero full slots
    sem_init(&full, 0, 0);
}

int insert_item(buffer_item item) {
    // Buffer full; can't insert
    if (size == BUFFER_SIZE) {
        return -1;
    }
    buffer[in] = item;
    // If we've reached slot 4, start back at 0
    in = (in + 1) % BUFFER_SIZE;
    size++;
    return 0;
}

int remove_item(buffer_item *item) {
    // Buffer empty, nothing to remove
    if (size == 0) {
        return -1;
    }
    buffer_item val = buffer[out];
    memcpy(item, &val, sizeof val);
    // If we've reached slot 4, start back at 0
    out = (out + 1) % BUFFER_SIZE;
    size--;
    return 0;
}
