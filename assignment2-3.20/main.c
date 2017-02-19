#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MIN_PID 300
#define MAX_PID 5000

#define BITMAP_SIZE (MAX_PID - MIN_PID) / 32 + 1

typedef struct {
    int nextAvailable;
    int bits[BITMAP_SIZE];
    pthread_mutex_t allocate_mutex;
    pthread_mutex_t release_mutex;
} Bitmap;

Bitmap bitmap;


#define GET_INDEX(k) ((k - MIN_PID) / 32);
#define GET_POS(k) ((k - MIN_PID) % 32);
#define NEXT_PID(k) ((k - MIN_PID) % (MAX_PID - MIN_PID) + MIN_PID + 1);
#define RANDRANGE(min, max) ((rand() - min) % (max - min + 1) + min)

void set_bit(int[], int);
void clear_bit(int[], int);
unsigned int test_bit(int[], int);
int allocate_map(void);
int allocate_pid(void);
int get_next_available(int);
void release_pid(int);
void *simulate_process(void *);

int main() {
    srand((unsigned int)time(NULL));
    allocate_map();
    int nThreads = 10000;
    pthread_t tid[nThreads];
    for (int i = 0; i < nThreads; i++) {
        pthread_create(&tid[i], NULL, simulate_process, NULL);
        //sleep(3);
    }
    for (int i = 0; i < nThreads; i++) {
        pthread_join(tid[i], NULL);
    }
    return 0;
}

void *simulate_process(void *vargp) {
    pthread_mutex_lock(&bitmap.allocate_mutex);
    int pid = allocate_pid();
    if (pid == -1) {
        printf("Insufficient PIDs available to create new process.\n");
        pthread_mutex_unlock(&bitmap.allocate_mutex);
        return NULL;
    }
    printf("Requesting PID %d\n", pid);
    pthread_mutex_unlock(&bitmap.allocate_mutex);
    unsigned int sleep_time = (unsigned)RANDRANGE(0, 10);
    sleep(sleep_time);
    pthread_mutex_lock(&bitmap.release_mutex);
    release_pid(pid);
    printf("Releasing PID %d\n", pid);
    pthread_mutex_unlock(&bitmap.release_mutex);
    return NULL;
}

void set_bit(int a[], int k) {
    int i = GET_INDEX(k);
    int pos = GET_POS(k);
    unsigned int flag = 1;
    flag = flag << pos;
    a[i] = a[i] | flag;
}

void clear_bit(int a[], int k) {
    int i = GET_INDEX(k);
    int pos = GET_POS(k);
    unsigned int flag = 1;
    flag = ~(flag << pos);
    a[i] = a[i] & flag;
}

unsigned int test_bit(int a[], int k) {
    if (k > MAX_PID || k < MIN_PID) {
        return 1;
    }
    int i = GET_INDEX(k);
    int pos = GET_POS(k);
    unsigned int flag = 1;
    flag = flag << pos;
    return a[i] & flag;
}

int allocate_map(void) {
    bitmap.nextAvailable = MIN_PID;
    void *a = memset(bitmap.bits, 0, sizeof(bitmap));
    if (a == NULL) {
        return -1;
    }
    return 1;
}

int get_next_available(int start) {
    int i = start;
    int counter = 0;
    if (test_bit(bitmap.bits, i)) {
        i = bitmap.nextAvailable + 1;
        while (1) {
            if (test_bit(bitmap.bits, i) == 0) {
                break;
            }
            if (counter == MAX_PID) {
                return -1;
            }
            i = NEXT_PID(i);
            counter++;
        }
    }
    return i;
}

int allocate_pid(void) {
    int next = get_next_available(bitmap.nextAvailable);
    if (next != -1) {
        set_bit(bitmap.bits, next);
    }
    bitmap.nextAvailable = RANDRANGE(MIN_PID, MAX_PID);
    return next;
}

void release_pid(int pid) {
    if (pid <= MAX_PID && pid >= MIN_PID && test_bit(bitmap.bits, pid)) {
        bitmap.nextAvailable = pid;
        clear_bit(bitmap.bits, pid);
    }
}