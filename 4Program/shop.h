//---------------------------------------------------------------------------
// shop.h
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

#ifndef SHOP_H_
#define SHOP_H_
#include <algorithm>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define kDefaultNumChairs 3  // the default number of chairs for waiting = 3
#define kDefaultBarbers 1   // the default number of barbers = 1

class Shop
{
 public:
  Shop(); // constructor
  Shop(int num_barbers, int num_chairs); // constructor, calls init

  int visitShop(int id); // customer visit shop. return barber ID or -1 (not served)
  void leaveShop(int customer_id, int barber_id); // customer leave shop
  void helloCustomer(int id); // barber greets customer
  void byeCustomer(int id); // barber finishes hair-cut service
  int getCustDrops() const; // get number of customers that left w/out service

private:
 void init(); // initialize conditions
 string int2string(int i); // converts integers to strings
 void printBarber(int person, string message); // print for barber threads
 void printCustomer(int person, string message); // print for customer threads

 const int max_waiting_cust_; // the max number of customers that can wait
 const int num_barbers; // number of barbers

 int cust_drops_; // number of customers that left w/out service

 /* vector index directly correlates to barber id */
 vector<bool> in_service_; // holds bools of customer in service at barber's chair
 vector<bool> money_paid_; // holds bools of customer paid barber
 vector<int>  customer_in_chair_; // holds id of customer in barber's chair

 queue<int> waiting_chairs_;  // includes the IDs of all waiting customer threads


 /* Mutexes and condition variables to coordinate threads */
 pthread_mutex_t mutex_; // mutex for all critical sections
 pthread_cond_t  cond_customers_waiting_; // customers waiting condition
 vector<pthread_cond_t> cond_barber_sleeping_; // barber sleeping condition
 vector<pthread_cond_t> cond_customer_served_; // customer serverd condition
 vector<pthread_cond_t> cond_barber_paid_;     // barber paid condition
};
#endif
