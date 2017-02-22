#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MIN_PID 300
#define MAX_PID 5000
#define NUM_SLOTS (MAX_PID - MIN_PID)

// Minimum number of bits needed to hold 4700 PIDs
#define BITMAP_SIZE (NUM_SLOTS) / 32 + 1

typedef struct {
    int nextAvailable;
    int bits[BITMAP_SIZE];
    pthread_mutex_t mutex;
} Bitmap;

Bitmap bitmap;

#define GET_INDEX(k) ((k - MIN_PID) / 32);
#define GET_POS(k) ((k - MIN_PID) % 32);
// Loop back to 300 once 5000 is reached
#define NEXT_PID(k) ((k - MIN_PID) % (NUM_SLOTS) + MIN_PID + 1);
// Random number in range [min, max]
#define RANDRANGE(min, max) ((rand() - min) % (max - min + 1) + min)

void set_bit(int[], int);
void clear_bit(int[], int);
unsigned int test_bit(int[], int);
int allocate_map(void);
int allocate_pid(void);
int get_next_available(int);
void release_pid(int);

int main(int argc, char *argv[]) {
    int a = allocate_pid();
    printf("allocating PID %d\n", a);
    int b = allocate_pid();
    printf("allocating PID %d\n", b);
    int c = allocate_pid();
    printf("allocating PID %d\n", c);

    release_pid(a);
    printf("releasing PID %d\n", a);
    release_pid(b);
    printf("releasing PID %d\n", b);
    release_pid(c);
    printf("releasing PID %d\n", c);
    
    return 0;
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
    // Return 0 if bit isn't set, else return n >= 1
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
    // Start at PID 300
    bitmap.nextAvailable = MIN_PID;
    void *a = memset(bitmap.bits, 0, sizeof(bitmap));
    if (a == NULL) {
        return -1;
    }
    return 1;
}

int get_next_available(int start) {
    int next = start;
    // Search for the next available slot
    if (test_bit(bitmap.bits, start)) {
        for (int counter = 0; counter < NUM_SLOTS; counter++) {
            next = NEXT_PID(next);
            if (test_bit(bitmap.bits, next) == 0) {
                break;
            }
            counter++;
        }
    }
    return next;
}

int allocate_pid(void) {
    int next = get_next_available(bitmap.nextAvailable);
    if (next != -1) {
        set_bit(bitmap.bits, next);
    }
    // The current slot is now used up, so choose a new random spot to check next
    bitmap.nextAvailable = RANDRANGE(MIN_PID, MAX_PID);
    return next;
}

void release_pid(int pid) {
    if (pid <= MAX_PID && pid >= MIN_PID && test_bit(bitmap.bits, pid)) {
        bitmap.nextAvailable = pid;
        clear_bit(bitmap.bits, pid);
    }
}
