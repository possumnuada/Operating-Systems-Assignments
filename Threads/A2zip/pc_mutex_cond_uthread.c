/* Possum Nuada
 *
 * Resources:
 * https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
 */
 #include <stdlib.h>
 #include <stdio.h>
 #include <assert.h>
 #include "uthread.h"
 #include "uthread_mutex_cond.h"
 #include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

uthread_mutex_t mutex;
uthread_cond_t cond_not_full;
uthread_cond_t cond_not_empty;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;


/*
 * This function is executed by a producer thread.
 * The function increments items when a mutex lock has been obtained and items
 * is less than MAX_ITEMS. It also adds to the histogram.
 */
void* producer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {

    // Obtain mutex lock
    uthread_mutex_lock( mutex );

    // While full, wait til not full
    while(items >= MAX_ITEMS){
      producer_wait_count++;
      uthread_cond_wait( cond_not_full);
    }

    uthread_cond_signal(cond_not_empty);

    items++;
    histogram[items] ++;
    uthread_mutex_unlock( mutex );

    //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );

  }
  return NULL;
}

/*
 * This function is executed by a consumer thread.
 * The function decrements items when a mutex lock has been obtained and items
 * is greater than zero. It also adds to the histogram.
 */
void* consumer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {

    uthread_mutex_lock( mutex );

    while(items == 0){
      consumer_wait_count++;
      uthread_cond_wait(cond_not_empty);
    }

    uthread_cond_signal(cond_not_full);

    items--;
    histogram[items] ++;
    uthread_mutex_unlock( mutex );

  //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );

  }
  return NULL;
}

int main() {
  uthread_t t[4];

  uthread_init (4);
  mutex = uthread_mutex_create();
  cond_not_full = uthread_cond_create(mutex);
  cond_not_empty = uthread_cond_create(mutex);

  t[0] = uthread_create(producer, "Producer 1");
  t[1] = uthread_create(producer, "Producer 1");
  t[2] = uthread_create(consumer, "Consumer 1");
  t[3] = uthread_create(consumer, "Consumer 2");

  uthread_join(t[0], NULL);
  uthread_join(t[1], NULL);
  uthread_join(t[2], NULL);
  uthread_join(t[3], NULL);

  uthread_cond_destroy(cond_not_full);
  uthread_cond_destroy(cond_not_empty);

  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf("Final number of items: %d\n", items);
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
