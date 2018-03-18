#include <pthread.h>
#include <iostream>
#include <stdio.h>


/* For safe condition variable usage, must use a boolean predicate and   */
/* a mutex with the condition.                                           */
int                 conditionMet = 0;
pthread_cond_t      cond[5];

pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

#define NTHREADS    5

void *threadfunc(void *parm)
{

  int i;
  long my_id = (long)parm;
  int           rc;

  int count=0;

  while(count++<3) // This line makes no sense, no repeatation as expected. 
  {
    rc = pthread_mutex_lock(&mutex);
    while ( conditionMet != my_id) 
    {
      printf("Thread %d blocked\n",my_id);
      rc = pthread_cond_wait(&cond[my_id], &mutex);
      printf("Thread %d unblocked\n", my_id);
    }

    rc = pthread_mutex_unlock(&mutex);     
    rc = pthread_mutex_lock(&mutex);
    conditionMet = (my_id+1)%NTHREADS ; // This is important to wait for next time
    rc = pthread_cond_signal(&cond[(my_id+1)%NTHREADS]);
    rc = pthread_mutex_unlock(&mutex);

   }
   return NULL;
}

int main(int argc, char **argv)
{
  int                   rc=0;
  int                   i;
  pthread_t             threadid[NTHREADS];

  for(rc=0;rc<NTHREADS;rc++)
    cond[rc]= PTHREAD_COND_INITIALIZER;

  printf("Enter Testcase - %s\n", argv[0]);

  printf("Create %d threads\n", NTHREADS);
  for(i=0; i<NTHREADS; ++i) {
    rc = pthread_create(&threadid[i], NULL, threadfunc, (void *)i);
    printf("Thread created=%d\n", i);
  }

  sleep(5);  /* Sleep is not a very robust way to serialize threads */
  rc = pthread_mutex_lock(&mutex);

  /* The condition has occured. Set the flag and wake up any waiting threads */
  conditionMet = 1;
  printf("Wake up all waiting threads...\n");
  rc = pthread_cond_signal(&cond[0]);    
  rc = pthread_mutex_unlock(&mutex);      

  printf("Wait for threads and cleanup\n");
  for (i=0; i<NTHREADS; ++i) {
    rc = pthread_join(threadid[i], NULL);        
  }
  pthread_cond_destroy(&cond[0]);
  pthread_mutex_destroy(&mutex);

  printf("Main completed\n");
  return 0;
}
