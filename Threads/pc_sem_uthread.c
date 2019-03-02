/* Possum Nuada
 * V00853812
 *
 * Resources:
 * http://www.csc.villanova.edu/~mdamian/threads/posixsem.html
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

uthread_sem_t sem_not_full;
uthread_sem_t sem_not_empty;
uthread_sem_t sem_critical;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_sem_wait(sem_not_full);
    uthread_sem_wait(sem_critical);

    items++;
    histogram[items]++;

    uthread_sem_signal(sem_not_empty);
    uthread_sem_signal(sem_critical);

  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_sem_wait(sem_not_empty);
    uthread_sem_wait(sem_critical);

    items--;
    histogram[items]++;

    uthread_sem_signal(sem_not_full);
    uthread_sem_signal(sem_critical);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);

  sem_not_full = uthread_sem_create(10);
  sem_not_empty = uthread_sem_create(0);
  sem_critical = uthread_sem_create(1);

  t[0] = uthread_create(producer, "Producer 1");
  t[1] = uthread_create(producer, "Producer 1");
  t[2] = uthread_create(consumer, "Consumer 1");
  t[3] = uthread_create(consumer, "Consumer 2");

  uthread_join(t[0], NULL);
  uthread_join(t[1], NULL);
  uthread_join(t[2], NULL);
  uthread_join(t[3], NULL);

  uthread_sem_destroy(sem_not_full);
  uthread_sem_destroy(sem_not_empty);
  uthread_sem_destroy(sem_critical);

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf("Final number of items: %d\n", items);
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
