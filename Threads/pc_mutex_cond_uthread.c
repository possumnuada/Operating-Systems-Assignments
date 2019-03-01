/* Possum Nuada
 * V00853812
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

    // Read items to check if it's likely the thread will be able to produce
    while(items>=MAX_ITEMS);

    // Obtain mutex lock
    uthread_mutex_lock( mutex );
  //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );

    // Check items again now that mutex lock has been obtained
    if(items >= MAX_ITEMS){
      // Give up lock and decrement counter if thread can't produce
      uthread_mutex_unlock( mutex );
      i--;
      producer_wait_count++;
    }else{
      // Produce, add to histogram, then give up lock
      items++;
      histogram[items] ++;
      uthread_mutex_unlock( mutex );
    }
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
    while(items==0);
    uthread_mutex_lock( mutex );
  //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );
    if(items < 1){
      uthread_mutex_unlock( mutex );
      i--;
      consumer_wait_count++;
    }else{
      items--;
      histogram[items] ++;
      uthread_mutex_unlock( mutex );
    }
  }
  return NULL;
}

int main() {
  uthread_t t[4];

  uthread_init (4);
   mutex = uthread_mutex_create();

  // TODO: Create Threads and Join

  t[0] = uthread_create(producer, "Producer 1");
  t[1] = uthread_create(producer, "Producer 1");
  t[2] = uthread_create(consumer, "Consumer 1");
  t[3] = uthread_create(consumer, "Consumer 2");

  uthread_join(t[0], NULL);
  uthread_join(t[1], NULL);
  uthread_join(t[2], NULL);
  uthread_join(t[3], NULL);

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
