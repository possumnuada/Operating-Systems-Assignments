/**
 * The Smoker's Problem solution by Possum Nuada
 * Agent code was provided, see "smoke.c".
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

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

// Condition variables to signal smokers
uthread_cond_t cond_has_paper;
uthread_cond_t cond_has_tobacco;
uthread_cond_t cond_has_match;

int signalled[5];  // Stores which resources were signalled this iteration

// wait_(resource) and has_(resource) functions increment waiting once they have 
// got the mutex and are about to wait on a condition variable.
// The agent is started once all 6 of those threads are waiting.
int waiting = 0;   

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This function sets the signalled values back to 0.
 */
void clear_signalled(){
  signalled[PAPER] = 0;
  signalled[TOBACCO] = 0;
  signalled[MATCH] = 0;
}

/** 
 * For debugging, prints the values of the signalled array.
 */
void print_signalled(){
  printf("Paper: %d, Tobacco: %d, Match %d\n",signalled[PAPER], signalled[TOBACCO], signalled[MATCH]);
}

/**
 * This function is called when the agent is finished.
 * It sends signals so that waiting threads can finish.
 */
void clean_up(struct Agent* a){
  uthread_mutex_lock (a->mutex);
  uthread_cond_signal(cond_has_paper);
  uthread_cond_signal(cond_has_tobacco);
  uthread_cond_signal(cond_has_match);
  uthread_cond_signal (a->paper);
  uthread_cond_signal (a->tobacco);
  uthread_cond_signal (a->match);
  uthread_mutex_unlock (a->mutex);
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

 // while(waiting < 6);
  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      //printf("%d\n", iteration);
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
    }
  uthread_mutex_unlock (a->mutex);
  
  return NULL;
}


/**
 *            *** Smoker Thread Functions - has_(resource) ***
 * 
 * These functions wait until signalled to smoke by the signal_smoker thread,
 * then they smoke, signal that they have smoked and start waiting again.
 */
void* has_paper(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(cond_has_paper);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [PAPER]++;
    uthread_cond_signal(a->smoke);
    // printf("has_paper smoked\n");
    uthread_cond_wait(cond_has_paper);
  }
  uthread_mutex_unlock(a->mutex);
  //printf("has_paper returned\n");
}

void* has_tobacco(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(cond_has_tobacco);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [TOBACCO]++;
    uthread_cond_signal(a->smoke);
    // printf("has_tobacco smoked\n");
    uthread_cond_wait(cond_has_tobacco);
  }
  uthread_mutex_unlock(a->mutex);
  // printf("has_tobacco returned\n");
}

void* has_match(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(cond_has_match);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [MATCH]++;
    uthread_cond_signal(a->smoke);
    // printf("has_match smoked\n");
    uthread_cond_wait(cond_has_match);
  }
  uthread_mutex_unlock(a->mutex);
  // printf("has_match returned\n");
}

/** 
 *            *** Wait Thread Functions - wait_(resource) ***
 * 
 * These functions wait until the resource they are waiting on is signalled available by 
 * the agent, then they update the signalled array to indicate their resource is available.
 */
void* wait_paper(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(a->paper);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw paper\n");
    signalled[PAPER] = 1;
    uthread_cond_wait(a->paper);
  }
  uthread_mutex_unlock(a->mutex);
  // printf("wait_paper returned\n");
}

void* wait_tobacco(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(a->tobacco);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw tobacco\n");
    signalled[TOBACCO] = 1;
    uthread_cond_wait(a->tobacco);
  }
  uthread_mutex_unlock (a->mutex);
  // printf("wait_tobacco returned\n");
}

void* wait_match(void* av){
  struct Agent* a = av;
  uthread_mutex_lock (a->mutex);
  waiting++;
  uthread_cond_wait(a->match);
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw match\n");
    signalled[MATCH] = 1;
    uthread_cond_wait(a->match);
  }
  uthread_mutex_unlock(a->mutex);
  // printf("wait_match returned\n");
}

/** 
 *            *** Signal Smoker Thread Function ***
 * 
 * This function checks the signalled array to see when there are two available resources 
 * and then signals the apropriate smoker that they can smoke, and clears the signalled array.
 */
void* signal_smoker(void* av){
  struct Agent* a = av;
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    if ( signalled[PAPER] + signalled[TOBACCO] + signalled[MATCH] == 2) {
      uthread_mutex_lock (a->mutex);
      // printf("signal_count[PAPER]: %d, smoke_count[PAPER]: %d ", signal_count [PAPER], smoke_count[PAPER]);
      // printf("signal_count[MATCH]: %d, smoke_count[MATCH]: %d ", signal_count [MATCH], smoke_count[MATCH]);
      // printf("signal_count[TOBACCO]: %d, smoke_count[TOBACCO]: %d \n", signal_count [TOBACCO], smoke_count[TOBACCO]);
      if(signalled[PAPER] == 0) {
        uthread_cond_signal(cond_has_paper);
      } else if(signalled[TOBACCO] == 0) {
        uthread_cond_signal(cond_has_tobacco);
      } else {
        uthread_cond_signal(cond_has_match);
      }
      // print_signalled();
      clear_signalled();
      // print_signalled();
      uthread_mutex_unlock(a->mutex);
    }
  }
}


int main (int argc, char** argv) {
  clear_signalled();
  uthread_init (8);
  struct Agent*  a = createAgent();

  uthread_t t[7];

  cond_has_paper = uthread_cond_create(a->mutex);
  cond_has_tobacco = uthread_cond_create(a->mutex);
  cond_has_match = uthread_cond_create(a->mutex);

  t[0] = uthread_create(has_paper, a);
  t[1] = uthread_create(has_tobacco, a);
  t[2] = uthread_create(has_match, a);
  t[3] = uthread_create(wait_paper, a);
  t[4] = uthread_create(wait_tobacco, a);
  t[5] = uthread_create(wait_match, a);
  t[6] = uthread_create(signal_smoker, a);

  // Wait for threads to start waiting on a signal before starting agent
  while(waiting < 6);

  // Start agent
  uthread_join (uthread_create (agent, a), 0);

  // Clean up threads waiting on signals once agent is finished
  clean_up(a);
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
