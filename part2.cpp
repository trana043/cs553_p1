#include <stdio.h>
#include <mpfr.h>
#include <gmp.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <iostream>
#include <pthread.h>

/* For safe condition variable usage, must use a boolean predicate and   */
/* a mutex with the condition.                                           */
int                 conditionMet = 0;

pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

#define NTHREADS    40
#define BASE 2
pthread_cond_t      cond[NTHREADS];
struct Message {
  int des;
  int source;
  mpz_t message;
  mpfr_t message2;
};

struct my_struct{
	mpz_t num1;
	mpfr_t num2;
};

int job = 50 * NTHREADS;

int debug = 1;
std::vector<std::vector<Message> > my_array;

std::vector<my_struct> evc1;


std::map<int, int> nprimes;
void print_out_array(){
   int j = 0;
   int i = 0;
   mpfr_t tmp1;
   mpfr_t tmp2;
   mpz_t tmp3;
   mpz_t tmp4;
   mpz_init(tmp3);
   mpz_init(tmp4);

   mpfr_init(tmp1);
   mpfr_init(tmp2);
   int true_pos = 0;
   int true_neg = 0;
   int fail_pos = 0;
   int fail_neg = 0;
   printf("here");
   printf("%d\n",evc1.size());
   while (i < evc1.size()) {
	   j = i;
	   while (j < evc1.size()){
		   mpfr_set(tmp1, (evc1[i]).num2,MPFR_RNDD );
		   mpfr_set(tmp2, (evc1[j]).num2,MPFR_RNDD );
		   if (mpfr_cmp(tmp1, tmp2) > 0)
			   mpfr_sub(tmp1,tmp1,tmp2, MPFR_RNDD);
		   else
			   mpfr_sub(tmp1,tmp2,tmp1, MPFR_RNDD);
		   mpfr_floor(tmp2, tmp1);

		   mpfr_sub(tmp1,tmp1,tmp2,MPFR_RNDD);
		   mpfr_mul_ui(tmp1,tmp1,100,MPFR_RNDD);

           mpz_set(tmp3,(evc1[i]).num1);
           mpz_set(tmp4,(evc1[j]).num1);
           if (mpz_cmp(tmp3, tmp4) > 0)
                mpz_mod(tmp3,tmp3,tmp4);
           else
                 mpz_mod(tmp3,tmp4,tmp3);
           if (mpfr_cmp_ui(tmp1,5) < 0 || mpfr_cmp_ui(tmp1,95) > 0){
                 if (mpz_cmp_ui(tmp3, 0) ==0)
                   	true_pos  ++;
                 else
                   	 fail_pos++;

            }else{
                 if (mpz_cmp_ui(tmp3, 0) >0)
                   	  true_neg  ++;
                 else
                   	  fail_neg++;

            }
		   j++;
	   }
	   i++;

   }
   printf("%d %d %d \n",true_pos, true_neg, true_pos+ true_neg);
   printf("%d %d %d\n",fail_pos, fail_neg,fail_pos+ fail_neg);

}


void *threadfunc(void *parm)
{
  int i=0;
  long my_id = (long)parm;
  int           rc;
  int count=0;
  int behavior = 0;
  Message tmp;
  mpfr_init(tmp.message2);
  mpz_init(tmp.message);


  mpz_t evc;
  mpz_init(evc);
  mpz_set_ui(evc,0);
  mpz_add_ui(evc,evc,nprimes[my_id]);

  int base = nprimes[my_id];
  mpz_t k;
  mpz_init(k);

  mpz_t tmp_evc;
  mpz_init(tmp_evc);
  mpz_set_ui(tmp_evc,0);

  mpfr_t my_time;

  mpfr_t log_evc;
  mpfr_t recv_log_evc;
  mpfr_init2 (log_evc, 200);
  mpfr_init2 (recv_log_evc, 200);

  mpfr_set_d (log_evc, 0.0, MPFR_RNDD);
  mpfr_set_d (recv_log_evc, 0.0, MPFR_RNDD);

  mpfr_t base2;
  mpfr_init (base2);
/*
  mpfr_exp_t exp;
  void* signicant;
  mpfr_prec_t prec = 4;
  mpfr_custom_init(signicant, prec);
  mpfr_custom_init_set(log_evc, MPFR_REGULAR_KIND,exp,prec, signicant);
*/

  mpz_t antilog1;
  mpz_t antilog2;
  mpz_init(antilog2);
  mpz_init(antilog1);
  mpfr_t tmp3;
  mpfr_t tmp_log_evc;
  mpfr_init2 (tmp3, 200);
  mpfr_init2 (tmp_log_evc, 200);

  mpfr_set_ui(tmp3, nprimes[my_id], MPFR_RNDD);
  mpfr_log2(base2, tmp3, MPFR_RNDD); // log(pi)
  while(1)
  {

     rc = pthread_mutex_lock(&mutex);
     while ( conditionMet != my_id)
     {
       rc = pthread_cond_wait(&cond[my_id], &mutex);
     }
     rc = pthread_mutex_unlock(&mutex);
     rc = pthread_mutex_lock(&mutex);

     srand (time(NULL));
     if (behavior < 2){ //internal
       job --;
       mpz_mul_ui(evc,evc,base);
       mpfr_add(log_evc, log_evc, base2, MPFR_RNDD); // log(encod) + log(pi)
       //printf("%d %d internal\n", my_id, job);


     } else if (behavior == 2){ //sending
       job--;
	   //printf("%d first %d\n", my_id,behavior);

       srand (time(NULL));
       int recv = rand() % NTHREADS;
       if (my_id == recv){
         recv= (recv+1)%NTHREADS;
       }
	   //printf("%d first %d\n", my_id,behavior);

       tmp.des = recv;
       tmp.source = my_id;
       mpz_init(tmp.message);
       mpz_mul_ui(evc,evc,base);  // n = n*base
       mpz_set(tmp.message,evc);

       //set the log
       mpfr_add(log_evc, log_evc, base2, MPFR_RNDD);
       mpfr_set(tmp.message2, log_evc,MPFR_RNDD);
       (my_array[recv]).push_back(tmp);



       my_struct my_tmp_struct;
       mpfr_init(my_tmp_struct.num2);
       mpfr_set(my_tmp_struct.num2, log_evc,MPFR_RNDD);

       mpz_init(my_tmp_struct.num1);
       mpz_set(my_tmp_struct.num1,evc);
       //mpz_out_str(stdout, 10,my_tmp_struct.num1);
       //printf("\nhere\n");
       evc1.push_back( my_tmp_struct);

     }



     if (job <=0){
        //printf("done %d %d\n", my_id, job);
        //printf("%d %d\n", my_id, job);
        //printf("%d overflow\n", my_id);
        print_out_array();
        return NULL;
     }

     while ( i<(my_array[my_id]).size()) {
        //printf("%d %d receving \n", my_id, job);


        job--;
        tmp = (my_array[my_id])[i];
        (my_array[my_id]).pop_back();

        // get the evc from mesage
        mpz_set(tmp_evc,tmp.message);
        mpz_lcm(evc,evc,tmp_evc);
        mpz_mul_ui(evc,evc,base);

        i++;

        mpz_t antilog1;
        mpz_t antilog2;
        mpfr_set(recv_log_evc, tmp.message2,MPFR_RNDD);

        mpfr_set(tmp_log_evc,log_evc,MPFR_RNDD);
        mpfr_exp2(tmp_log_evc, tmp_log_evc, GMP_RNDN);

        mpfr_add(log_evc, log_evc, recv_log_evc, MPFR_RNDD);//ti = s + ti
        mpfr_exp2(recv_log_evc, recv_log_evc, GMP_RNDN);

        mpfr_get_z(antilog1, recv_log_evc, GMP_RNDN);
        mpfr_get_z(antilog2, log_evc, GMP_RNDN);


        mpz_gcd(antilog1, antilog2, antilog1);
        mpfr_set_ui(tmp3, 0, MPFR_RNDD);

        while (1){
           mpfr_add_ui(tmp3,tmp3,1,MPFR_RNDD);
           if (mpfr_cmp_z(tmp3, antilog1)==0){
              break;
           }
        }
        //tmp3 = gcd of anti log s and anti log t

        mpfr_log2(recv_log_evc, tmp3, MPFR_RNDD);
        mpfr_sub(log_evc, log_evc, recv_log_evc, MPFR_RNDD);
        mpfr_add(log_evc, log_evc, base2, MPFR_RNDD);
        //printf("%d\n", my_id);
        //mpfr_out_str (stdout, 10,0,log_evc,MPFR_RNDD );
        //printf("\n");
        //mpz_out_str (stdout, 10,evc );
        //printf("\n");


        my_struct my_tmp_struct;
        mpfr_init(my_tmp_struct.num2);
        mpfr_set(my_tmp_struct.num2, log_evc,MPFR_RNDD);
        mpz_init(my_tmp_struct.num1);
        mpz_set(my_tmp_struct.num1,evc);
		//mpz_out_str(stdout, 10,my_tmp_struct.num1);
        //printf("\nhere\n");
        evc1.push_back( my_tmp_struct);


        if (job <=0){
          //printf("%d %d\n", my_id, job);
          //printf("%d overflow\n", my_id);
        	  print_out_array();
          return NULL;
	    }
     }
    // (my_array[my_id]).clear();
     behavior = (behavior+1)%3;


     conditionMet = (my_id+1)%NTHREADS ; // This is important to wait for next time
     int t = (my_id+1)%NTHREADS;
     rc = pthread_cond_signal(&cond[t]);
     rc = pthread_mutex_unlock(&mutex);

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
