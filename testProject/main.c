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

void *write_(void *);
void *read_(void *);
void *write2(void *);
void *read2(void *);

sem_t empty;
sem_t full;
pthread_mutex_t mutex;
pthread_mutex_t rw_mutex;
pthread_mutex_t wrt;
pthread_mutex_t mutexW;
pthread_mutex_t rd;
int wc = 0;
int read_count = 0;

int main(int argc, char *argv[]) {
    unsigned int sleep_time = 10;
    int n = 10;

    // Dummy tid. Don't actually need to use this since pthread_join isn't used.
    pthread_t tid;
    for (int i = 0; i < n; i++) {
        pthread_create(&tid, NULL, read2, NULL);
        pthread_create(&tid, NULL, write2, NULL);
    }
    sleep(sleep_time);
    return 0;
}

void *write_(void *params) {
    long int tid = syscall(SYS_gettid);
    while(1) {
        sleep(2);
        pthread_mutex_lock(&rw_mutex);
        printf("writing\n");
        sleep(1);
        pthread_mutex_unlock(&rw_mutex);
    }
}

void *read_(void *params) {
    long int tid = syscall(SYS_gettid);
    while(1) {
        sleep(1);
        pthread_mutex_lock(&mutex);
        read_count++;
        if (read_count == 1) {
            pthread_mutex_lock(&rw_mutex);
        }
        pthread_mutex_unlock(&mutex);
        // read
        printf("%d reading\n", read_count);
        sleep(1);
        pthread_mutex_lock(&mutex);
        read_count--;
        if (read_count == 0) {
            pthread_mutex_unlock(&rw_mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *write2(void *params) {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&rd);
        pthread_mutex_lock(&mutex);
        read_count++;
        if (read_count == 1) {
            pthread_mutex_lock(&wrt);
        }
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&rd);
        // read
        printf("reading %d\n", read_count);
        pthread_mutex_lock(&mutex);
        read_count--;
        if (read_count == 0) {
            pthread_mutex_unlock(&wrt);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *read2(void *params) {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&mutexW);
        wc++;
        if (wc == 1) {
            pthread_mutex_lock(&rd);
        }
        pthread_mutex_unlock(&mutexW);
        pthread_mutex_lock(&wrt);
        // write
        printf("writing %d\n", wc);
        pthread_mutex_unlock(&wrt);
        pthread_mutex_lock(&mutexW);
        wc--;
        if (wc == 0) {
            pthread_mutex_unlock(&rd);
        }
        pthread_mutex_unlock(&mutexW);
    }
}