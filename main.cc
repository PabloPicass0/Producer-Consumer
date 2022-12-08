#include "helper.h"
#include <iostream>

using namespace std;

extern int errno;

//Declares semaphores
sem_t semEmpty, semFull, semMtx;

void *producer (void *ptr_prod);
void *consumer (void *ptr_cons);


int main (int argc, char **argv)
{
  //Reads in inputs and checks if valid; otherwise returns 22 (errno invalid arguments)
  if (!validInput(argc, argv)) {
    return 22;
  }

  int size_queue = atoi(argv[1]);
  int num_jobs = atoi(argv[2]);
  int num_prod = atoi(argv[3]);
  int num_cons = atoi(argv[4]);

  //Creates circular buffer using the imported library in the header file
  cb circ_buff(size_queue);
  for (int count = 0; count < size_queue; count++) {
    circ_buff[count] = nullptr;
  }

  //Initializes semaphores
  if (sem_init(&semEmpty, 0, size_queue) < 0 ) {
    perror("Error msg");
    return errno;
  }
  if (sem_init(&semFull, 0, 0) < 0 ) {
    perror("Error msg");
    return errno;
  }
  if (sem_init(&semMtx, 0, 1) < 0) {
    perror("Error msg");
    return errno;
  }
   
  //Sets up array of producers; one thread for each producer
  pthread_t pt_array[num_prod];
  for (int count = 0; count < num_prod; count++) {
    prod_struct* ptr_prod =  new prod_struct {count + 1, num_jobs, &circ_buff};

    if (pthread_create(&pt_array[count], NULL, producer, ptr_prod) != 0) {
      printf("Error msg: producer thread %u could not be created", count + 1);
      return 1;
    }
  }
  
  //Sets up array of consumers; one thread for each consumer
  pthread_t ct_array[num_cons]; 
  for (int count = 0; count < num_cons; count++) {
    cons_struct* ptr_cons = new cons_struct {count + 1, &circ_buff};

    if (pthread_create(&ct_array[count], NULL, consumer, ptr_cons) != 0) {
      printf("Error msg: consumer thread %u could not be created", count + 1);
      return 1;
    }
  }

  //Joins producer and consumer threads (waits until they are terminated)
   for (int count = 0; count < num_prod; count++) {
     if (pthread_join(pt_array[count], NULL) != 0) {
       printf("Error msg: producer thread %u could not be joined", count + 1);
       return 1;
     }
   }

   for (int count = 0; count < num_cons; count++) {
      if (pthread_join(ct_array[count], NULL) != 0) {
        printf("Error msg: consumer thread %u could not be joined", count + 1);
	return 1;
      }
    }
   
  //Destroys semaphores because process is finished; ensures clean-up
    if (sem_destroy(&semEmpty) < 0) {
      perror("Error msg");
      return errno;
    }
    if (sem_destroy(&semFull) < 0) {
      perror("Error msg");
      return errno;
    }
    if (sem_destroy(&semMtx) < 0) {
      perror("Error msg");
      return errno;
    }
  
  return 0;
}


void *producer (void *ptr_prod)
{
  prod_struct* ptr = (prod_struct*) ptr_prod;
  timespec ts;

  while (ptr->num_jobs > 0) {
    //Creates new job
    job_struct* ptr_job = new job_struct;
    ptr_job->duration = (rand() % 10) + 1;
    ptr->num_jobs -= 1;

    //If no empty space opens up in the circular queue, thread is exited after waiting 20 sec 
    ts.tv_sec = time(NULL) + 20;
    if (sem_timedwait(&semEmpty, &ts) < 0) {
      printf("Producer(%u): fell asleep waiting for an empty space\n", ptr->prod_id);
      delete ptr_job;
      delete ptr;
      pthread_exit(0);
    }

    //Enters critical section to add the job
    if (sem_wait(&semMtx) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }
    
    ptr_job->id = ptr->queue->size();
    ptr->queue->push_back(ptr_job);

    //Unlocks critical section and increases the "semFull" counter by one
    if (sem_post(&semMtx) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }
    if (sem_post(&semFull) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }

    printf("Producer(%u): job id %u duration %u\n",
	   ptr->prod_id, ptr_job->id, ptr_job->duration);

    sleep((rand() % 5) + 1);
  }

  //Exits thread once all jobs have been created
  printf("Producer(%u): No more jobs to generate\n", ptr->prod_id);
  delete ptr;
  pthread_exit(0);

}

void *consumer (void *ptr_cons) 
{
  cons_struct* ptr = (cons_struct*) ptr_cons;
  job_struct* ptr_job = NULL;
  timespec ts;
  int id, duration;

  while (true) {
    //If no more jobs appear in the circular queue, thread is exited after waiting 20 sec 
    ts.tv_sec = time(NULL) + 20;
    if (sem_timedwait(&semFull, &ts) < 0) {
      printf("Consumer(%u): No more jobs left.\n", ptr->cons_id);
      delete ptr;
      pthread_exit(0);
    }

    //Enters critical section remove a job
    if (sem_wait(&semMtx) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }
    
    ptr_job = ptr->queue->front();
    id = ptr_job->id; 
    duration = ptr_job->duration;
    ptr->queue->pop_front();

    //Unlocks critical section and increases the "semEmpty" counter by one
    if (sem_post(&semMtx) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }
    if (sem_post(&semEmpty) < 0) {
      perror("Error msg");
      pthread_exit(0);
    }

    printf("Consumer(%u): job id %u executing sleep duration %u\n",
	   ptr->cons_id, id, duration);

    sleep(duration);

    printf("Consumer(%u): job id %u completed\n", ptr->cons_id, id);
  }
}
