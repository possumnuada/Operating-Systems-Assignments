/* Possum Nuada
 * V00853812
 *
 * Resources:
 * http://www.csc.villanova.edu/~mdamian/threads/posixsem.html
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

sem_t sem_not_full;
sem_t sem_not_empty;
sem_t sem_critical;

void* producer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    sem_wait(&sem_not_full);
    sem_wait(&sem_critical);

    items++;
    histogram[items]++;

  //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );

    sem_post(&sem_not_empty);
    sem_post(&sem_critical);
  }
  return NULL;
}

void* consumer (void* v) {
  char* thread;
  thread = (char*) v;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    sem_wait(&sem_not_empty);
    sem_wait(&sem_critical);

    items--;
    histogram[items]++;

  //  printf("Thread: %s, Iteration: %d, Number of Items: %d \n", thread, i, items );

    sem_post(&sem_not_full);
    sem_post(&sem_critical);
  }
  return NULL;
}

int main (int argc, char** argv) {
  pthread_t t[4];
  int  p1_status, p2_status, c1_status, c2_status;
  char *p1 = "Producer 1";
  char *p2 = "Producer 2";
  char *c1 = "Consumer 1";
  char *c2 = "Consumer 2";

  sem_init(&sem_not_full, 0, 10);
  sem_init(&sem_not_empty, 0, 0);
  sem_init(&sem_critical, 0, 1);

  /* Create Produces */

  p1_status = pthread_create(&t[0], NULL, producer, (void*) p1);
  p2_status = pthread_create(&t[1], NULL, producer, (void*) p2);

  /* Create Comsumers */

  c1_status = pthread_create(&t[2], NULL, consumer, (void*) c1);
  c2_status = pthread_create(&t[3], NULL, consumer, (void*) c2);

  printf("Producer thread 1 returns: %d\n",p1_status);
  printf("Producer thread 2 returns: %d\n",p2_status);
  printf("Consumer thread 1 returns: %d\n",p1_status);
  printf("Consumer thread 2 returns: %d\n",p2_status);

  pthread_join( t[0], NULL);
  pthread_join( t[1], NULL);
  pthread_join( t[2], NULL);
  pthread_join( t[3], NULL);

  sem_destroy(&sem_not_full);
  sem_destroy(&sem_not_empty);
  sem_destroy(&sem_critical);

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf("Final number of items: %d\n", items);
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);

  exit(0);
}
