#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs     3
#define kDefaultNumBarbers    1

class Shop_org 
{
public:

   // take in the number of barbers and use the default otherwise
   // we will need to adjust the constructor to fill and size the array
   Shop_org(int num_chairs) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0)
   { 
      init(); 
   };


   Shop_org() : max_waiting_cust_(kDefaultNumChairs), customer_in_chair_(0), in_service_(false),
      money_paid_(false), cust_drops_(0)
   { 
      init(); 
   };

   bool visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int id);
   void helloCustomer();
   void byeCustomer();
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_;              // the max number of threads that can wait

   // this is the customer id of the one customer in the barber chair being
   // serviced

   // for many barbers this will be the size of the number of barbers
   int customer_in_chair_;


   // this is a flag used to indicate that the one chair the barber
   // services is occupied by a patron getting their hair cut

   // for many barbers this will be the size of the number of barbers
   bool in_service_;            

   // for many barbers this will be the size of the number of barbers
   bool money_paid_;

   queue<int> waiting_chairs_;  // includes the ids of all waiting threads

   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  cond_customer_served_;
   pthread_cond_t  cond_barber_paid_;
   pthread_cond_t  cond_barber_sleeping_;

   static const int barber = 0; // the id of the barber thread
  
   void init();
   string int2string(int i);
   void print(int person, string message);
};
#endif
