/**
 * The Smoker's Problem solution by Possum Nuada
 * Agent code was provided, see "smoke.c".
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  pthread_mutex_init(&(agent->mutex), NULL);
  pthread_cond_init(&(agent->paper), NULL);
  pthread_cond_init(&(agent->match), NULL);
  pthread_cond_init(&(agent->tobacco), NULL);
  pthread_cond_init(&(agent->smoke), NULL);
  return agent;
}

// Condition variables to signal smokers
pthread_cond_t cond_has_paper   = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_has_tobacco = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_has_match   = PTHREAD_COND_INITIALIZER;

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
  pthread_mutex_lock (&(a->mutex));
  pthread_cond_signal(&cond_has_paper);
  pthread_cond_signal(&cond_has_tobacco);
  pthread_cond_signal(&cond_has_match);
  pthread_cond_signal (&(a->paper));
  pthread_cond_signal (&(a->tobacco));
  pthread_cond_signal (&(a->match));
  pthread_mutex_unlock (&(a->mutex));
}

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = (struct Agent*) av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

 // while(waiting < 6);
  pthread_mutex_lock (&(a->mutex));
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      // printf("%d\n", i);
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        // printf("match available\n");
        pthread_cond_signal (&(a->match));
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        // printf("paper available\n" );
        pthread_cond_signal (&(a->paper));
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        // printf("tobacco available\n");
        pthread_cond_signal (&(a->tobacco));
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      // printf("agent is waiting for smoker to smoke\n");
      pthread_cond_wait (&(a->smoke), &(a->mutex));
    }
  pthread_mutex_unlock (&(a->mutex));
  
  return NULL;
}


/**
 *            *** Smoker Thread Functions - has_(resource) ***
 * 
 * These functions wait until signalled to smoke by the signal_smoker thread,
 * then they smoke, signal that they have smoked and start waiting again.
 */
void* has_paper(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&cond_has_paper, &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [PAPER]++;
    pthread_cond_signal(&(a->smoke));
    // printf("has_paper smoked\n");
    pthread_cond_wait(&cond_has_paper, &(a->mutex));
  }
  pthread_mutex_unlock(&(a->mutex));
  //printf("has_paper returned\n");
}

void* has_tobacco(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&cond_has_tobacco, &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [TOBACCO]++;
    pthread_cond_signal(&(a->smoke));
    // printf("has_tobacco smoked\n");
    pthread_cond_wait(&cond_has_tobacco, &(a->mutex));
  }
  pthread_mutex_unlock(&(a->mutex));
  // printf("has_tobacco returned\n");
}

void* has_match(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&cond_has_match, &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    smoke_count [MATCH]++;
    pthread_cond_signal(&(a->smoke));
    // printf("has_match smoked\n");
    pthread_cond_wait(&cond_has_match, &(a->mutex));
  }
  pthread_mutex_unlock(&(a->mutex));
  // printf("has_match returned\n");
}

/** 
 *            *** Wait Thread Functions - wait_(resource) ***
 * 
 * These functions wait until the resource they are waiting on is signalled available by 
 * the agent, then they update the signalled array to indicate their resource is available.
 */
void* wait_paper(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&(a->paper), &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw paper\n");
    signalled[PAPER] = 1;
    pthread_cond_wait(&(a->paper), &(a->mutex));
  }
  pthread_mutex_unlock(&(a->mutex));
  // printf("wait_paper returned\n");
}

void* wait_tobacco(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&(a->tobacco), &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw tobacco\n");
    signalled[TOBACCO] = 1;
    pthread_cond_wait(&(a->tobacco), &(a->mutex));
  }
  pthread_mutex_unlock (&(a->mutex));
  // printf("wait_tobacco returned\n");
}

void* wait_match(void* av){
  struct Agent* a = (struct Agent*) av;
  pthread_mutex_lock (&(a->mutex));
  waiting++;
  pthread_cond_wait(&(a->match), &(a->mutex));
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    // printf("Saw match\n");
    signalled[MATCH] = 1;
    pthread_cond_wait(&(a->match), &(a->mutex));
  }
  pthread_mutex_unlock(&(a->mutex));
  // printf("wait_match returned\n");
}

/** 
 *            *** Signal Smoker Thread Function ***
 * 
 * This function checks the signalled array to see when there are two available resources 
 * and then signals the apropriate smoker that they can smoke, and clears the signalled array.
 */
void* signal_smoker(void* av){
  struct Agent* a = (struct Agent*) av;
  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS){
    if ( signalled[PAPER] + signalled[TOBACCO] + signalled[MATCH] == 2) {
      pthread_mutex_lock(&(a->mutex));
      // printf("signal_count[PAPER]: %d, smoke_count[PAPER]: %d | ", signal_count [PAPER], smoke_count[PAPER]);
      // printf("signal_count[MATCH]: %d, smoke_count[MATCH]: %d | ", signal_count [MATCH], smoke_count[MATCH]);
      // printf("signal_count[TOBACCO]: %d, smoke_count[TOBACCO]: %d \n", signal_count [TOBACCO], smoke_count[TOBACCO]);
      if(signalled[PAPER] == 0) {
        pthread_cond_signal(&cond_has_paper);
      } else if(signalled[TOBACCO] == 0) {
        pthread_cond_signal(&cond_has_tobacco);
      } else {
        pthread_cond_signal(&cond_has_match);
      }
      // print_signalled();
      clear_signalled();
      // print_signalled();
      pthread_mutex_unlock(&(a->mutex));
    }
  }
}


int main (int argc, char** argv) {
  clear_signalled();

  struct Agent*  a = createAgent();

  pthread_t t[8];

  pthread_create(&t[0], NULL, has_paper, (void*) a);
  pthread_create(&t[1], NULL, has_tobacco, (void*) a);
  pthread_create(&t[2], NULL, has_match, (void*) a);
  pthread_create(&t[3], NULL, wait_paper, (void*) a);
  pthread_create(&t[4], NULL, wait_tobacco, (void*) a);
  pthread_create(&t[5], NULL, wait_match, (void*) a);
  pthread_create(&t[6], NULL, signal_smoker, (void*) a);

  // Wait for threads to start waiting on a signal before starting agent
  while(waiting < 6);

  // Start agent
  pthread_join (pthread_create (&t[7], NULL, &agent, (void*) a), NULL);

  while(smoke_count[PAPER] + smoke_count[TOBACCO] + smoke_count[MATCH] < NUM_ITERATIONS);
  

  // Clean up threads waiting on signals once agent is finished
  clean_up(a);
  pthread_join(t[0], NULL);
  pthread_join(t[1], NULL);
  pthread_join(t[2], NULL);
  pthread_join(t[3], NULL);
  pthread_join(t[4], NULL);
  pthread_join(t[5], NULL);
  pthread_join(t[6], NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}
