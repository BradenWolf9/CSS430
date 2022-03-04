//---------------------------------------------------------------------------
// shop.cpp
//---------------------------------------------------------------------------
// Acts as structure for barber threads and customer threads to interact through
// a monitor.
//
// Assumptions:
//  -- valid params sent to constructor
// Implementation
//  -- uses one mutex for all critical sections
//  -- uses one condition for customers waiting
//  -- uses a vector to store conditions for barbers sleeping, one per barber
//  -- uses a vector to store conditions for customer served, one per barber
//  -- uses a vector to store conditions for barber paid, one per barber
//---------------------------------------------------------------------------

#include "shop.h"

Shop::Shop() : num_barbers{kDefaultBarbers},
max_waiting_cust_{kDefaultNumChairs} // constructor
{
  cust_drops_ = 0;

  in_service_.resize(num_barbers, false);
  money_paid_.resize(num_barbers, false);
  customer_in_chair_.resize(num_barbers,0);

  cond_barber_sleeping_.resize(num_barbers);
  cond_customer_served_.resize(num_barbers);
  cond_barber_paid_.resize(num_barbers);

  init();
}

Shop::Shop(int num_barbers, int num_chairs) : num_barbers{num_barbers},
max_waiting_cust_{num_chairs} // constructor, calls init
{
  cust_drops_ = 0;

  in_service_.resize(num_barbers, false);
  money_paid_.resize(num_barbers, false);
  customer_in_chair_.resize(num_barbers,0);

  cond_barber_sleeping_.resize(num_barbers);
  cond_customer_served_.resize(num_barbers);
  cond_barber_paid_.resize(num_barbers);

  init();
}

void Shop::init() // initialize conditions
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   for (int i = 0; i < cond_barber_sleeping_.size(); i++) {
     pthread_cond_init(&cond_barber_sleeping_[i], NULL);
     pthread_cond_init(&cond_customer_served_[i], NULL);
     pthread_cond_init(&cond_barber_paid_[i], NULL);
   }
}



int Shop::visitShop(int id) // customer visit shop. return barber ID or -1 (not served)
{
   pthread_mutex_lock(&mutex_);

   // If all waiting and service chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_ && *min_element(customer_in_chair_.begin(), customer_in_chair_.end()) > 0)
   {
      printCustomer(id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   // while all barbers are busy
   while (*min_element(customer_in_chair_.begin(), customer_in_chair_.end()) > 0)
   {
      waiting_chairs_.push(id);
      printCustomer(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }


   int barber_id; // declare out of for loop so can use in future cond_barber_sleeping
   // find barber with empty service chair
   for (barber_id = 0; barber_id < num_barbers; barber_id++)
   {
     if (customer_in_chair_[barber_id] == 0)
     {
       break;
     }
   }

   printCustomer(id, "moves to a service chair[" + int2string(barber_id) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[barber_id] = id;
   in_service_[barber_id] = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_[barber_id]);

   pthread_mutex_unlock(&mutex_);
   return barber_id;
}

void Shop::leaveShop(int customer_id, int barber_id) // customer leave shop
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   printCustomer(customer_id, "wait for barber[" + int2string(barber_id) + "] to be done with hair-cut");
   while (in_service_[barber_id] == true)
   {
      pthread_cond_wait(&cond_customer_served_[barber_id], &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_[barber_id] = true;
   pthread_cond_signal(&cond_barber_paid_[barber_id]);
   printCustomer(customer_id, "says good-bye to barber[" + int2string(barber_id) + "]");
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int id) // barber greets customer
{
   pthread_mutex_lock(&mutex_);

   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_[id] == 0 )
   {
      printBarber(id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   if (customer_in_chair_[id] == 0) // check if the customer sit down.
   {
       pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }
   
   printBarber(id, "starts a hair-cut service for customer[" + int2string(customer_in_chair_[id]) + "]");
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int id) // barber finishes hair-cut service
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[id] = false;
  printBarber(id, "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_[id]) + "]");
  money_paid_[id] = false;
  pthread_cond_signal(&cond_customer_served_[id]);
  while (money_paid_[id] == false)
  {
      pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_[id] = 0;
  printBarber(id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}

int Shop::getCustDrops() const // get number of customers that left w/out service
{
  return cust_drops_;
}

string Shop::int2string(int i) // converts integers to strings
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::printBarber(int person, string message) // print for barber threads
{
  cout << "barber  [" << person << "]: " << message << endl;
}

void Shop::printCustomer(int person, string message) // print for customer threads
{
  cout << "customer[" << person << "]: " << message << endl;
}
