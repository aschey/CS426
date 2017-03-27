#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

typedef enum { ASLEEP, BUSY } TA_enum;
typedef enum { PROGRAMMING, SEEKING_HELP } student_enum;

struct Student {
    int num;
    student_enum state;
};

#define QUEUE_SIZE 3
#define STUDENT_SLEEP_MIN 2
#define STUDENT_SLEEP_MAX 6
#define TA_SLEEP_MIN 0
#define TA_SLEEP_MAX 2

#define RANDRANGE(min, max) ((rand() - min) % (max - min + 1) + min)

struct Student *waiting_queue[QUEUE_SIZE];
int queue_size;
TA_enum TA_state;

pthread_mutex_t mutex;
pthread_mutex_t student_mutex;
pthread_mutex_t queue_mutex;

sem_t wakeup;
sem_t finished;

void *student(void *);
void *TA(void *);
int can_wait(void);
void add_student(struct Student*);
struct Student *get_next_student(void);
char *get_queue(void);
void rand_sleep(void);
void TA_sleep(void);

int main(int argc, char *argv[]) {
    // Seed random() function
    srand((unsigned)time(NULL));

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&student_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);

    sem_init(&wakeup, 0, 0);
    sem_init(&finished, 0, 0);

    TA_state = ASLEEP;

    if (argc < 2) {
        printf("Number of students required.\n");
        return 0;
    }
    int num_students = atoi(argv[1]);

    pthread_t tid;
    pthread_create(&tid, NULL, TA, NULL);
    for (int i = 0; i < num_students; i++) {
        int *num = malloc(sizeof(*num));
        *num = i;
        pthread_create(&tid, NULL, student, num);
    }

    sleep((unsigned)(num_students * 5));
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
            printf("Student %d is waking up the TA.\n", student_num);
            add_student(&cur_student);

            // Wake up TA
            sem_post(&wakeup);
            // Wait until TA is finished helping
            sem_wait(&finished);

            cur_student.state = PROGRAMMING;
            pthread_mutex_unlock(&student_mutex);
            rand_sleep();
        }
        else {
            // TA is busy
            pthread_mutex_lock(&queue_mutex);
            if (can_wait()) {
                add_student(&cur_student);
                printf("Student %d is now in the waiting queue.\nQueue size: %d\n"
                               "Queue value: %s", student_num, queue_size, get_queue());
                pthread_mutex_unlock(&queue_mutex);
                continue;
            }
            else {
                // No seats available
                printf("All seats are full. Student %d will come back later.\nQueue size: %d\n"
                               "Queue value: %s", student_num, queue_size, get_queue());
            }
            pthread_mutex_unlock(&queue_mutex);
        }
        cur_student.state = PROGRAMMING;
        printf("Student %d is programming.\n", student_num);
    }

}

void *TA(void *args) {
    while (1) {
        TA_sleep();
        if (queue_size == 0) {
            TA_state = ASLEEP;
            printf("The TA is sleeping.\n");
            // Wait until student wakes TA up
            sem_wait(&wakeup);
        }

        TA_state = BUSY;
        pthread_mutex_lock(&queue_mutex);
        struct Student *next_student = get_next_student();
        pthread_mutex_unlock(&queue_mutex);
        printf("The TA is helping student %d.\nQueue size: %d\n"
                       "Queue value: %s", next_student->num, queue_size, get_queue());
        TA_sleep();
        next_student->state = PROGRAMMING;
        printf("The TA finished helping student %d.\n", next_student->num);
        // Tell student that TA is finished
        sem_post(&finished);
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
    if (queue_size == 0) {
        return NULL;
    }
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
    sleep((unsigned)RANDRANGE(STUDENT_SLEEP_MIN, STUDENT_SLEEP_MAX));
}

void TA_sleep() {
    sleep((unsigned)RANDRANGE(TA_SLEEP_MIN, TA_SLEEP_MAX));
}
