#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define NUMBER_OF_THREADS 5
#define MAX_COUNT 500000

static sem_t obj_produced;
static sem_t obj_consumed;

static int shelf;
FILE *File;

void * producer() {
    int i;

    for(i=2; i< MAX_COUNT; i++) {
        shelf = i;
        sem_post(&obj_produced);
        sem_wait(&obj_consumed);
    }

    return NULL;
}


void * consumer() {
    unsigned char isPrime;
    int i;
    int VUT;
    int primes = 0;

    while(1) {
        sem_wait(&obj_produced);
        VUT = shelf;
        sem_post(&obj_consumed);
        //printf("[CONSUMER] %d\n", VUT);

        isPrime = 1;
        for (i=2;i<VUT; i++) {
            if (VUT % i ==0) {
                isPrime = 0;
            }
        }
        if(isPrime==1) {
            //printf("    thread #x announces that %d is prime.\n", i);
            primes++;
        }
    }
}


int main(void) {
    for (int n = 2; n < 300; n++) {
        double mean_t = 0;
        for (int test = 0; test < 3; test++) {
            int i = 0, err;
            pthread_t tid[n];
            // create semaphores
            err = sem_init(&obj_produced, 0, 0);
            if (err != 0) {
                printf("\ncan't create semaphore: obj_produced [%s]", strerror(err));
                return 1;
            }
            err = sem_init(&obj_consumed, 0, 0);
            if (err != 0) {
                printf("\ncan't create semaphore: obj_produced [%s]", strerror(err));
                return 1;
            }

            //start time
            clock_t t;
            t = clock();
            // create producer thread
            err = pthread_create(&(tid[i]), NULL, &producer, NULL);
            if (err != 0) {
                printf("\ncan't create producer thread: [%s]", strerror(err));
                return 1;
            }
            printf("Producer thread created\n");

            // create consumer threads
            for (i = 1; i < n; i++) {
                err = pthread_create(&(tid[i]), NULL, &consumer, NULL);
                if (err != 0) {
                    printf("\ncan't create consumer thread %d: [%s]", i, strerror(err));
                }
                printf("Consumer thread %d created\n", i);
            }

            // wait for producer thread
            pthread_join(tid[0], NULL);

            // kill consumer threads
            for (i = 1; i < n; i++) {
                pthread_kill(tid[i], 9);
            }
            //stop time
            t = clock() - t;
            double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
            mean_t += time_taken;
            // delete the semaphores
            sem_destroy(&obj_produced);
            sem_destroy(&obj_consumed);
        }
        printf("avg: %fs\n",mean_t/3);
        File = fopen("output.txt", "a");
        fprintf(File, "%d,%f",n,mean_t/3);
        fclose(File);
    }
    return 0;
}