#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;


void* threadFunction1(void* arg) {
    pthread_mutex_lock(&mutexA);
    printf("Thread 1 acquired lock A\n");
   
  
    usleep(1000000);
   
    printf("Thread 1 waiting for lock B\n");
    pthread_mutex_lock(&mutexB);
    printf("Thread 1 acquired lock B\n");

    pthread_mutex_unlock(&mutexB);
    pthread_mutex_unlock(&mutexA);
    return NULL;
}


void* threadFunction2(void* arg) {
    pthread_mutex_lock(&mutexB);
    printf("Thread 2 acquired lock B\n");


    usleep(1000000);

    printf("Thread 2 waiting for lock A\n");
    pthread_mutex_lock(&mutexA);
    printf("Thread 2 acquired lock A\n");

    pthread_mutex_unlock(&mutexA);
    pthread_mutex_unlock(&mutexB);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;


    pthread_create(&thread1, NULL, threadFunction1, NULL);
    pthread_create(&thread2, NULL, threadFunction2, NULL);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program finished\n");

    return 0;
}