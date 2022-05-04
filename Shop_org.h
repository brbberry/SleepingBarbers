#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultCustomers 10
#define kDefaultNumBarbers 1

class Shop_org
{
public:
   // take in the number of barbers and use the default otherwise
   // we will need to adjust the constructor to fill and size the array
   // both of the arrays were originally 0
   Shop_org(int num_customers, int num_chairs, int num_barbers) : max_waiting_cust_((num_chairs > 0) ? num_chairs : kDefaultNumChairs),
                                                                  customer_in_chair_{nullptr},
                                                                  in_service_{nullptr}, 
                                                                  money_paid_{nullptr},
                                                                  cust_drops_(0)
   {
      int numBarbarbers = (num_barbers > 0) ? num_barbers : kDefaultNumBarbers;
      // intialize the shop to know how many customers there are
      int numCustomers = (num_customers > 0) ? num_customers : kDefaultCustomers;

      // +1 to allow for barber ID to be used as signal catcher
      customer_in_chair_ = new int[numBarbarbers+1];
      in_service_ = new bool[numBarbarbers+1];
      money_paid_ = new bool[numBarbarbers+1];
      init(numCustomers, numBarbarbers);
   };

   Shop_org() : max_waiting_cust_(kDefaultNumChairs), customer_in_chair_(0), in_service_{nullptr},
                money_paid_{nullptr}, cust_drops_(0)
   {
      customer_in_chair_ = new int[kDefaultNumBarbers+1];
      in_service_ = new bool[kDefaultNumBarbers+1];
      money_paid_ = new bool[kDefaultNumBarbers+1];
      init(kDefaultCustomers, kDefaultNumBarbers);
   };

   ~Shop_org()
   {
      delete customer_in_chair_;
      customer_in_chair_ = nullptr;
      delete in_service_;
      in_service_ = nullptr;
      delete money_paid_;
      money_paid_ = nullptr;
   }
   
   int visitShop(int id); // return true only when a customer got a service
   void leaveShop(int id, int barberID);
   void helloCustomer(int id);
   void byeCustomer(int id);
   int get_cust_drops() const;

private:
   const int max_waiting_cust_; // the max number of threads that can wait

   // this is the customer id of the one customer in the barber chair being
   // serviced

   // for many barbers this will be the size of the number of barbers
   int *customer_in_chair_;

   // this is a flag used to indicate that the one chair the barber
   // services is occupied by a patron getting their hair cut

   // for many barbers this will be the size of the number of barbers
   bool *in_service_;

   // for many barbers this will be the size of the number of barbers
   bool *money_paid_;

   queue<int> waiting_chairs_; // includes the ids of all waiting threads

   // will be used by the customer to signal the sleeping barber
   queue<int> barbers_ready_; // includes the ids of all sleeping barbers

   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   // we want to make this an array so that we can send a signal specifically
   // to the thread waiting at the front of the Q

   // indexed with customer ID and will only be as large as the number of customers
   pthread_cond_t cond_customers_waiting_;
   pthread_cond_t *cond_customer_served_;
   pthread_cond_t *cond_barber_paid_;
   // we want to make this an array so that we can send a signal specifically
   // to the thread waiting at the front of the Q
   pthread_cond_t *cond_barber_sleeping_;

   static const int barber = 0; // the id of the barber thread

   void init(int numCustomers, int numBarbers);
   string int2string(int i);
   void print(int person, string message);
};
#endif
