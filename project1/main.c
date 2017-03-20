#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef enum { ASLEEP, BUSY } TA_enum;
typedef enum { PROGRAMMING, SEEKING_HELP } student_enum;

struct Student {
    int num;
    student_enum state;
};

#define QUEUE_SIZE 3
#define SLEEP_MIN 2
#define SLEEP_MAX 6
#define TA_MIN 0
#define TA_MAX 2

#define RANDRANGE(min, max) ((rand() - min) % (max - min + 1) + min)

struct Student *waiting_queue[QUEUE_SIZE];
int queue_size;
TA_enum TA_state;
pthread_cond_t wakeup;
pthread_cond_t finished_helping;
pthread_mutex_t mutex;
pthread_mutex_t student_mutex;
pthread_mutex_t queue_mutex;

int finished = 0;
int wakeup_check = 0;

// 3 chairs
// No students, nap
// check for more students when finished helping
// Awaken TA if asleep (use semaphore)
// wait in chair if TA busy
// leave if no chairs available

void *student(void *);
void *TA(void *);
int can_wait(void);
void add_student(struct Student*);
struct Student *get_next_student(void);
char *get_queue(void);
void rand_sleep(void);
void TA_sleep(void);

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL));
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&student_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    TA_state = ASLEEP;
    int num_students = atoi(argv[1]);

    pthread_t tid;
    pthread_create(&tid, NULL, TA, NULL);
    for (int i = 0; i < num_students; i++) {
        int *num = malloc(sizeof(*num));
        *num = i;
        pthread_create(&tid, NULL, student, num);
    }
    sleep(30);
    return 0;
}

void *student(void *args) {
    int *num = args;
    int student_num = *num;
    struct Student cur_student;
    cur_student.num = student_num;
    cur_student.state = PROGRAMMING;
    while (1) {
        rand_sleep();
        if (cur_student.state == SEEKING_HELP) {
            continue;
        }
        cur_student.state = SEEKING_HELP;
        printf("Student %d is seeking help.\n", student_num);

        if (TA_state == ASLEEP) {
            pthread_mutex_lock(&student_mutex);
            pthread_mutex_lock(&mutex);
            printf("Student %d is waking up the TA.\n", student_num);
            // Wake up
            add_student(&cur_student);
            wakeup_check = 1;
            pthread_cond_broadcast(&wakeup);
            while (!finished) {
                pthread_cond_wait(&finished_helping, &mutex);
            }
            finished = 0;
            printf("The TA finished helping student %d.\n", student_num);
            pthread_mutex_unlock(&student_mutex);
            pthread_mutex_unlock(&mutex);
        }
        else {
            // TA is busy
            pthread_mutex_lock(&queue_mutex);
            if (can_wait()) {
                add_student(&cur_student);
                printf("Student %d is now in the waiting queue.\nQueue size: %d\n"
                               "Queue value: %s", student_num, queue_size, get_queue());
                pthread_mutex_unlock(&queue_mutex);
            }
            else {
                // No seats available
                printf("All seats are full. Student %d will come back later.\nQueue size: %d\n"
                               "Queue value: %s", student_num, queue_size, get_queue());
                rand_sleep();
                cur_student.state = PROGRAMMING;
                printf("Student %d is programming.\n", student_num);
            }
            pthread_mutex_unlock(&queue_mutex);
        }
    }

}

void *TA(void *args) {
    while (1) {
        TA_sleep();
        pthread_mutex_lock(&queue_mutex);
        if (queue_size == 0) {
            TA_state = ASLEEP;
            printf("The TA is sleeping.\n");
            while (!wakeup_check) {
                pthread_cond_wait(&wakeup, &mutex);
            }
            wakeup_check = 0;
        }
        TA_state = BUSY;
        struct Student *next_student = get_next_student();
        if (next_student == NULL) {
            printf("NULL\n");
        }
        pthread_mutex_unlock(&queue_mutex);
        printf("The TA is helping student %d.\nQueue size: %d\n"
                       "Queue value: %s", next_student->num, queue_size, get_queue());
        rand_sleep();
        next_student->state = PROGRAMMING;
        printf("Student %d is programming.\n", next_student->num);
        finished = 1;
        pthread_cond_broadcast(&finished_helping);
    }

}

int can_wait() {
    return queue_size < 3;
}

void add_student(struct Student *student) {
    waiting_queue[queue_size] = student;
    queue_size++;
}

struct Student *get_next_student() {
    struct Student *next = waiting_queue[0];
    memmove(&waiting_queue[0], &waiting_queue[1], sizeof(waiting_queue) -  sizeof(*waiting_queue));
    queue_size--;
    return next;
}

char* get_queue() {
    char *queue = malloc(sizeof(char) * 128);
    int index = 0;
    for (int i = 0; i < queue_size; i++) {
       index += sprintf(&queue[index], "%d ", waiting_queue[i]->num);
    }
    queue[index] = '\n';
    return queue;
}

void rand_sleep() {
    sleep((unsigned)RANDRANGE(SLEEP_MIN, SLEEP_MAX));
}

void TA_sleep() {
    sleep((unsigned)RANDRANGE(TA_MIN, TA_MAX));
}