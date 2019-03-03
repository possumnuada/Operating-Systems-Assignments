/* Possum Nuada
 *
 * Resources:
 * https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  cond_not_empty  = PTHREAD_COND_INITIALIZER;

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
    pthread_mutex_lock( &mutex );

    // While full, wait til not full
    while(items >= MAX_ITEMS){
      producer_wait_count++;
      pthread_cond_wait( &cond_not_full, &mutex );
    }

    pthread_cond_signal(&cond_not_empty);

    items++;
    histogram[items] ++;
    pthread_mutex_unlock( &mutex );
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

    pthread_mutex_lock( &mutex );

    while(items == 0){
      consumer_wait_count++;
      pthread_cond_wait(&cond_not_empty, &mutex);
    }

    pthread_cond_signal(&cond_not_full);

    items--;
    histogram[items] ++;
    pthread_mutex_unlock( &mutex );
  }
  return NULL;
}

int main() {
  pthread_t t[4];
  int  p1_status, p2_status, c1_status, c2_status;
  char *p1 = "Producer 1";
  char *p2 = "Producer 2";
  char *c1 = "Consumer 1";
  char *c2 = "Consumer 2";

  /* Create Produces */

  p1_status = pthread_create( &t[0], NULL, producer, (void*) p1);
  p2_status = pthread_create( &t[1], NULL, producer, (void*) p2);

  /* Create Comsumers */

  c1_status = pthread_create( &t[2], NULL, consumer, (void*) c1);
  c2_status = pthread_create( &t[3], NULL, consumer, (void*) c2);


  /* Wait till threads are complete before main continues. Unless we  */
  /* wait we run the risk of executing an exit which will terminate   */
  /* the process and all threads before the threads have completed.   */

  pthread_join( t[0], NULL);
  pthread_join( t[1], NULL);
  pthread_join( t[2], NULL);
  pthread_join( t[3], NULL);

  printf("Producer thread 1 returns: %d\n",p1_status);
  printf("Producer thread 2 returns: %d\n",p2_status);
  printf("Consumer thread 1 returns: %d\n",p1_status);
  printf("Consumer thread 2 returns: %d\n",p2_status);

  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
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
