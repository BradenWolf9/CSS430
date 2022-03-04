#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "shop.h"
using namespace std;

void *barber(void *);
void *customer(void *);

// ThreadParam class
// a set of parameters to be passed to each thread
// used as a way to pass more than one argument to a thread
class ThreadParam
{
public:
  ThreadParam(Shop *shop, int id, int service_time) :
    shop(shop), id(id), service_time(service_time) { };
   Shop *shop;              // a pointer to the Shop object
   int id;                  // a thread identifier
   int service_time;        // service time (usec) for barber; 0 for customer
};

int main(int argc, char *argv[])
{
   // Read arguments from command line
   if (argc != 5)
   {
       cout << "Usage: num_barbers num_chairs num_customers service_time" << endl;
       return -1;
   }

   // convert params to integers
   int num_barbers = atoi(argv[1]);
   int num_chairs = atoi(argv[2]);
   int num_customers = atoi(argv[3]);
   int service_time = atoi(argv[4]);

   if (num_barbers < 1) // valid num barbers check
   {
      cout << "Invalid amount of barbers. Please enter integer greater than 0." << endl;
      return -1;
   }
   if (num_chairs < 0) // valid num chairs check
   {
     cout << "Invalid amount of chairs. Please enter integer greater than -1." << endl;
     return -1;
   }
   if (num_customers < 0)
   {
     cout << "Invalid amount of customers. Please enter an integer greater than -1." << endl;
     return -1;
   }
   if (service_time < 0)
   {
     cout << "Invalid service time. Please enter an integer greater than -1." << endl;
     return -1;
   }


   // many barbers, many customers, one shop
   pthread_t barber_threads[num_barbers];
   pthread_t customer_threads[num_customers];
   Shop shop(num_barbers, num_chairs);

   // create barber threads
   for (int i = 0; i < num_barbers; i++)
   {
     ThreadParam* barber_param = new ThreadParam(&shop, i, service_time);
     pthread_create(&barber_threads[i], NULL, barber, barber_param);
   }

   // create customer threads
   for (int i = 0; i < num_customers; i++)
   {
      usleep(rand() % 1000);
      int id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   // Wait for customers to finish
   for (int i = 0; i < num_customers; i++)
   {
       pthread_join(customer_threads[i], NULL);
   }

   /* gives barbers enough time to get to sleep state. If cancel was called
   when barber thread was not asleep, then the number of customers who didn't
   recieve a service would not be printed. */
   usleep(250000); //

   // cancel barber(s)
   for (int i = 0; i < num_barbers; i++)
   {
     pthread_cancel(barber_threads[i]);
   }

   usleep(500000); // sleep 0.5 seconds to allow barber threads to cancel fully

   cout << "# customers who didn't receive a service = " << shop.getCustDrops() << endl;
   return 0;
}

// the barber thread function
void *barber(void *arg)
{
  // extract parameters and delete param
  ThreadParam &param = *(ThreadParam *) arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  int service_time = param.service_time;
  delete &param;

  // keep working until being terminated by the main
  while(true)
  {
    shop.helloCustomer(id); // pick up a new customer
    usleep(service_time); // complete service by sleeping for service_time
    shop.byeCustomer(id); // release the customer
  }
  //return nullptr;
}

// the customer thread function
void *customer(void *arg)
{
  // extract parameters and delete param
  ThreadParam &param = *(ThreadParam *) arg;
  Shop &shop = *(param.shop);
  int id = param.id;
  delete &param;

  int barber = -1; // -1 means did not get barber
  // if assigned to barber i then wait for service to finish
  if ((barber = shop.visitShop(id)) != -1)
  {
    shop.leaveShop(id, barber); // wait until my service is finished
  }
  //return nullptr;
}
