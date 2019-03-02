#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 10

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
//
uthread_cond_t cond_has_paper;
uthread_cond_t cond_has_tobacco;
uthread_cond_t cond_has_match;
uthread_cond_t cond_resource;

int signalled[5]; // Stores which resources were signalled this iteration
int iteration;

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

void clear_signalled(){
  signalled[PAPER] = 0;
  signalled[TOBACCO] = 0;
  signalled[MATCH] = 0;
}

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};


  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        // printf("match available\n");
        uthread_cond_signal (a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        // printf("paper available\n" );
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        // printf("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      // printf("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
      iteration++;
      printf("%d\n", iteration);
    }
  uthread_cond_signal(cond_has_paper);
  uthread_cond_signal(cond_has_tobacco);
  uthread_cond_signal(cond_has_match);
  smoke_count [PAPER]--;
  smoke_count [TOBACCO]--;
  smoke_count [MATCH]--;
  uthread_mutex_unlock (a->mutex);
  uthread_mutex_lock (a->mutex);
  uthread_cond_signal (a->paper);
  uthread_cond_signal (a->tobacco);
  uthread_cond_signal (a->match);
  uthread_mutex_unlock (a->mutex);
  printf("hi\n");
  return NULL;
}

void* has_paper(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(cond_has_paper);
    uthread_cond_signal(a->smoke);
    smoke_count [PAPER]++;
    uthread_mutex_unlock(a->mutex);
  }
}

void* has_tobacco(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(cond_has_tobacco);
    uthread_cond_signal(a->smoke);
    smoke_count [TOBACCO]++;
    uthread_mutex_unlock(a->mutex);
  }
}

void* has_match(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(cond_has_match);
    uthread_cond_signal(a->smoke);
    smoke_count [MATCH]++;
    uthread_mutex_unlock(a->mutex);
  }
}

void* wait_paper(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(a->paper);
    // printf("Saw paper\n");
    signalled[PAPER] = 1;
    uthread_mutex_unlock(a->mutex);
  }
}

void* wait_tobacco(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(a->tobacco);
    // printf("Saw tobacco\n");
    signalled[TOBACCO] = 1;
    uthread_mutex_unlock(a->mutex);
  }
}

void* wait_match(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    uthread_mutex_lock (a->mutex);
    uthread_cond_wait(a->match);
  //  printf("Saw match\n");
    signalled[MATCH] = 1;
    uthread_mutex_unlock(a->mutex);
  }
}

void* signal_smoker(void* av){
  struct Agent* a = av;
  while(iteration < NUM_ITERATIONS){
    while (  !(signalled[PAPER] == 1 && signalled[TOBACCO] == 1)
          || !(signalled[PAPER] == 1 && signalled[MATCH] == 1)
          || !(signalled[MATCH] == 1 && signalled[TOBACCO] == 1)) {
      uthread_mutex_lock (a->mutex);
      if(signalled[PAPER] == 0) {
        uthread_cond_signal(cond_has_paper);
      } else if(signalled[TOBACCO] == 0) {
        uthread_cond_signal(cond_has_tobacco);
      } else {
        uthread_cond_signal(cond_has_match);
      }
      clear_signalled();
      uthread_mutex_unlock(a->mutex);
    }
  }
}

int main (int argc, char** argv) {
  clear_signalled();
  iteration = 0;
  uthread_init (8);
  struct Agent*  a = createAgent();

  uthread_t t[7];

  cond_has_paper = uthread_cond_create(a->mutex);
  cond_has_tobacco = uthread_cond_create(a->mutex);
  cond_has_match = uthread_cond_create(a->mutex);
  cond_resource = uthread_cond_create(a->mutex);

  t[0] = uthread_create(has_paper, a);
  t[1] = uthread_create(has_tobacco, a);
  t[2] = uthread_create(has_match, a);
  t[3] = uthread_create(wait_paper, a);
  t[4] = uthread_create(wait_tobacco, a);
  t[5] = uthread_create(wait_match, a);
  t[6] = uthread_create(signal_smoker, a);


  uthread_join (uthread_create (agent, a), 0);
  uthread_join(t[0], NULL);
  uthread_join(t[1], NULL);
  uthread_join(t[2], NULL);
  uthread_join(t[3], NULL);
  uthread_join(t[4], NULL);
  uthread_join(t[5], NULL);
  uthread_join(t[6], NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}
