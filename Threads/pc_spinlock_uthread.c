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

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
spinlock_t lock;

void* producer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {

    // Read items to check if it's likely the thread will be able to produce
    while(items>=MAX_ITEMS) producer_wait_count++;

    // Obtain lock
    spinlock_lock (&lock);

    // Check items again now that mutex lock has been obtained
    if(items >= MAX_ITEMS){
      // Give up lock and decrement counter if thread can't produce
      spinlock_unlock (&lock);
      i--;
      producer_wait_count++;
    }else{
      // Produce, add to histogram, then give up lock
      items++;
      histogram[items] ++;
      spinlock_unlock (&lock);
    }
    //printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );
  }
  return NULL;
}

void* consumer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    while(items==0) consumer_wait_count++;
    spinlock_lock (&lock);

    if(items < 1){
      spinlock_unlock (&lock);
      i--;
      consumer_wait_count++;
    }else{
      items--;
      histogram[items] ++;
      spinlock_unlock (&lock);
    }
    //printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);

  spinlock_create (&lock);

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
