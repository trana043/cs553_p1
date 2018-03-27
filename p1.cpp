#include <stdio.h>
#include <gmp.h>
#include <stdio.h>
#include <assert.h>
#include <queue>
#include <unistd.h>
#include <map>
#include <iostream>
#include <pthread.h>

/*http://www.cplusplus.com/forum/unices/116373/ pipe in C++*/

/* For safe condition variable usage, must use a boolean predicate and   */
/* a mutex with the condition.                                           */
int                 conditionMet = 0;

pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

#define NTHREADS    100
pthread_cond_t      cond[NTHREADS];
struct Message {
  int des;
  int source;
  mpz_t message;
};

int debug = 1;
std::vector<std::vector<Message> > my_array;
std::map<int, int> nprimes;
void *threadfunc(void *parm)
{
  int i;
  long my_id = (long)parm;
  int           rc;
  int count=0;
  int behavior = 0;
  Message tmp;

  mpz_t n;
  mpz_init(n);
  mpz_set_ui(n,0);
  mpz_add_ui(n,n,nprimes[my_id]);
  
  int base = nprimes[my_id];
  mpz_t k;
  mpz_init(k);
  int job_count = 0;
  while(1)
  {

    //printf("%d has", my_id);// job_count);
    rc = pthread_mutex_lock(&mutex);
    while ( conditionMet != my_id) 
    {
      //printf("Thread %d blocked\n",my_id);
      rc = pthread_cond_wait(&cond[my_id], &mutex);
      //printf("Thread %d unblocked\n", my_id);
    }
    rc = pthread_mutex_unlock(&mutex);     


     rc = pthread_mutex_lock(&mutex);
     //printf("Thread %d working\n", my_id);

     srand (time(NULL));
     if (behavior < 2){ //internal
        //printf("internaljob\n");
       job_count++;
       mpz_mul_ui(n,n,base);  
     } else if (behavior == 2){ //sending
       //printf("sending job\n");
       job_count++;
       srand (time(NULL));
       int recv = rand() % NTHREADS;
       if (my_id == recv){
         recv= (recv+1)%NTHREADS;
       }
       tmp.des = recv;
       tmp.source = my_id;
       mpz_init(tmp.message);
       mpz_mul_ui(n,n,base);  
       mpz_set(k,n);
       mpz_set(tmp.message,k);
       // printf ("%d send to %d\n", my_id, recv);
       (my_array[recv]).push_back(tmp);
       
     }
        //printf("%d\n", my_id);
        //mpz_out_str(stdout,10,k);
        //printf("\n");
        printf("%d %d\n", my_id, job_count);
     if (mpz_sizeinbase(n, 2) > 32 * NTHREADS){
        printf("%d %d\n", my_id, job_count);
        return NULL;
     }  
     //printf("%d done sending and internal\n", my_id); 
     //printf("%d , initial messge size is %d\n\n",my_id, (my_array[my_id]).size());
     
     while ( i<(my_array[my_id]).size()) {
        job_count++;
        tmp = (my_array[my_id])[i];
        (my_array[my_id]).pop_back();
        mpz_set(k,tmp.message);
        mpz_lcm(n,n,k);
        mpz_mul_ui(n,n,base); 
        //printf("%d\n", my_id);
        //mpz_out_str(stdout,10,n);
        //printf("\n\n");
        i++;
        if (mpz_sizeinbase(n, 2) > 32 * NTHREADS){
          printf("%d %d\n", my_id, job_count);
          printf("%d overflow\n", my_id);
          return NULL;
	}  
     }
     (my_array[my_id]).clear();
     //printf("%d done with msg\n", my_id);
     behavior = (behavior+1)%3;

     conditionMet = (my_id+1)%NTHREADS ; // This is important to wait for next time
     int t = (my_id+1)%NTHREADS;
     rc = pthread_cond_signal(&cond[t]);
     rc = pthread_mutex_unlock(&mutex);
     if (job_count >10000) {
        printf("\n\n\n\n\n\n%d has %d\n\n\n\n\n", my_id, job_count);
        break;
     }
  }
  return NULL;
}


int main(int argc, char **argv)
{
  int                   rc=0;
  int                   i;
  pthread_t             threadid[NTHREADS];

  // find the prime number
  int n = NTHREADS; //User input
  int num = 1; //number that will be tested if its a prime
  int primeCount = 0; //counts the times it num % i == 0


  //counts the number of primes
  int index = 0;
  while(1){
    num++;
    primeCount = 0;
    //Determines if a number is prime
    for(int i = 1; i <= num; i++)
    {

        if(num % i == 0)
        {
            primeCount++;
        }
    }
    if(primeCount == 2)
    {
       nprimes[index]=num;
       nprimes[num]=index++;
       std::vector<Message>myvec;
       printf("%d %d\n", n, num);
       my_array.push_back(myvec);
       n--;
       if (n == 0){ break;}
    }
  }
 
  for(rc=0;rc<NTHREADS;rc++)
    cond[rc]= (pthread_cond_t) PTHREAD_COND_INITIALIZER;

  printf("Enter Testcase - %s\n", argv[0]);

  printf("Create %d threads\n", NTHREADS);
  for(i=0; i<NTHREADS; ++i) {
    rc = pthread_create(&threadid[i], NULL, threadfunc, (void *)i);
    printf("Thread created=%d\n", i);
  }

  sleep(140);
  printf("Wait for threads and cleanup\n");
  for (i=0; i<NTHREADS; ++i) {
    rc = pthread_join(threadid[i], NULL);
  }
  pthread_cond_destroy(&cond[0]);
  pthread_mutex_destroy(&mutex);

  printf("Main completed\n");
  return 0;
}
