//See https://en.wikipedia.org/wiki/POSIX_Threads#Example
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <rastro.h>

#define NUM_THREADS 5

void *perform_work(void *arguments){
  int index = *((int *)arguments);
  rst_init(index, index);
  rst_event(3);
  int sleep_time = (1 + rand() % NUM_THREADS)*100;
  printf("THREAD %d: Started.\n", index);
  printf("THREAD %d: Will be sleeping for %d microseconds.\n", index, sleep_time);
  usleep(sleep_time/1000000);
  printf("THREAD %d: Ended.\n", index);
  rst_event(4);
  rst_finalize();
}

int main(void) {
  pthread_t threads[NUM_THREADS];
  int thread_args[NUM_THREADS];
  int i;
  int result_code;

  rst_init(NUM_THREADS+1, NUM_THREADS+1);
  rst_event(1);
  
  //create all threads one by one
  for (i = 0; i < NUM_THREADS; i++) {
    printf("IN MAIN: Creating thread %d.\n", i);
    thread_args[i] = i;
    result_code = pthread_create(&threads[i], NULL, perform_work, &thread_args[i]);
    assert(!result_code);
  }

  printf("IN MAIN: All threads are created.\n");

  //wait for each thread to complete
  for (i = 0; i < NUM_THREADS; i++) {
    result_code = pthread_join(threads[i], NULL);
    assert(!result_code);
    printf("IN MAIN: Thread %d has ended.\n", i);
  }

  printf("MAIN program has ended.\n");
  rst_event(2);
  rst_finalize();
  return 0;
}
