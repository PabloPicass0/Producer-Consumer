# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <iostream>
# include <semaphore.h>
# include <boost/circular_buffer.hpp>

using namespace std;

//Checks wether command line arguments are valid (5 in total and only digits)
bool validInput(int argc, char** argv);

//Job structs are filled into the circular queue by producers and deleted by consumers
struct job_struct {
  int id;
  int duration;
};

//Defines the type circular buffer as circular buffer of pointers to jobs
typedef boost::circular_buffer<job_struct*> cb;

//Defines the type of pointers which are used to access jobs in the circular buffer 
typedef boost::circular_buffer<job_struct*> *ptr_cb;

//Producer and consumer structs
struct prod_struct {
  int prod_id;
  int num_jobs;
  ptr_cb queue;
};

struct cons_struct {
  int cons_id;
  ptr_cb queue;
};
