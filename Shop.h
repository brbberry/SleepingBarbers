// Blake Berry
// 05/06/2022
// Homework 2
// This Program is a contract of the shop monitor. The monitor holds 
// a mutex protecting its general access along with condition variables that
// scale with the number of barbers. Various shared boolean values are 
// gaurded
//-----------------------------------------------------------------------------

#ifndef SHOP_H_
#define SHOP_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultCustomers 10
#define kDefaultNumBarbers 1

class Shop
{
public:


   //------------------------- Constructor ---------------------------------------
   // constructs a shop give the number of barbers, customer, and number of 
   // waiting chairs
   // Postconditions: A general shop monitor is created
   Shop(int num_customers, int num_chairs, int num_barbers) : 
      max_waiting_cust_((num_chairs >= 0) ? num_chairs : kDefaultNumChairs),
      customer_in_chair_{nullptr},
      in_service_{nullptr},
      money_paid_{nullptr},
      ready_to_serve{nullptr},
      cust_drops_(0)
   {
      int numBarbarbers    = (num_barbers >= 0) ? num_barbers : kDefaultNumBarbers;
      int numCustomers     = (num_customers >= 0) ? num_customers : kDefaultCustomers;

      // +1 to allow for barber ID to be used as signal catcher
      customer_in_chair_                     = new int[numBarbarbers + 1];
      in_service_                            = new bool[numBarbarbers + 1];
      money_paid_                            = new bool[numBarbarbers + 1];
      ready_to_serve                         = new bool[numBarbarbers + 1];
      ready_to_serve[numBarbarbers + 1]      = {0};

      init(numCustomers, numBarbarbers);
   };


  //------------------------- Constructor ---------------------------------------
   // constructs a shop using default number of barbers, customer, and number of 
   // waiting chairs
   // Postconditions: A general shop monitor is created
   Shop() : 
      max_waiting_cust_(kDefaultNumChairs), 
      customer_in_chair_(0), 
      in_service_{nullptr},
      money_paid_{nullptr},
      ready_to_serve{nullptr}, 
      cust_drops_(0)
   {
      customer_in_chair_                        = new int[kDefaultNumBarbers + 1];
      in_service_                               = new bool[kDefaultNumBarbers + 1];
      money_paid_                               = new bool[kDefaultNumBarbers + 1];
      ready_to_serve                            = new bool[kDefaultNumBarbers];
      ready_to_serve[kDefaultNumBarbers + 1]    = {0};

      init(kDefaultCustomers, kDefaultNumBarbers);
   };


   //------------------------- Destructor ---------------------------------------
   // Frees the dynamic memory assocaited with the monitors conditions and 
   // shared varaibles
   // Postconditions: the memory is returned to the system
   ~Shop()
   {
      delete customer_in_chair_;
      customer_in_chair_ = nullptr;
      delete in_service_;
      in_service_ = nullptr;
      delete money_paid_;
      money_paid_ = nullptr;
      delete ready_to_serve;
      ready_to_serve = nullptr;
      delete cond_customer_served_;
      cond_customer_served_ = nullptr;
      delete cond_barber_paid_;
      cond_barber_paid_ = nullptr;
      delete cond_barber_sleeping_;
      cond_barber_sleeping_ = nullptr;
   }


   //-------------------------- Visit Shop ---------------------------------------
   // A customer thread method that simulates a customer entering a barber shop.
   // The customer thread will leave the shop if there are not enough waiting 
   // chairs
   // preconditions : assumes the conditionals have been instantiated
   // Postconditions: prints the customer barber communication to the barber
   //                 leaves the customer in the barbers service chair
   //                 returns the ID of the barber cutting hair
   int visitShop(int id);


   //-------------------------- Leave Shop ---------------------------------------
   // A customer thread method that simulates a customer leaving a barber shop.
   // The customer thread pays the barber and then leaves the shop
   // preconditions : assumes the conditionals have been instantiated
   // Postconditions: signals to the barber it has paid and leaves the shop
   void leaveShop(int id, int barberID);

   //-------------------------- Hello Customer -----------------------------------
   // A barber thread method simulating the greeting of a customer for a haircut
   // preconditions : assumes the conditionals have been instantiated
   //                 assumes the customer enter shop method signals its arrival
   // Postconditions: the barber is not waiting and is ready to cut hair
   void helloCustomer(int id);


   //--------------------------Bye Customer -------------------------------------
   // A barber thread method simulating the finishing of a customer for a haircut
   // preconditions : assumes the conditionals have been instantiated
   //                 assumes the customer leave shop method signals to the barber
   //                 they have paid
   // Postconditions: singals to the customer theyre being served and that the
   //                 next customer can be serviced
   void byeCustomer(int id);


   //------------------------- get_cust_drops ---------------------------------------
   // returns the number of customers dropped
   // Postconditions: the number of customers not serviced is returned
   int get_cust_drops() const;

private:
   const int max_waiting_cust_; // the max number of threads that can wait

   // list of customers in the barbers chairs
   int *customer_in_chair_;

   // list of if the barbers are currently cutting hair
   bool *in_service_;

   // list of if the barbers clients paid
   bool *money_paid_;

   // bool array for barbers ready to cut hair
   bool *ready_to_serve;

   queue<int> waiting_chairs_; // includes the ids of all waiting threads

   // Q of barbers ready to cut hair
   queue<int> barbers_ready_;

   int cust_drops_;

   // Monitor Mutex
   pthread_mutex_t mutex_;

   // conditionals
   pthread_cond_t cond_customers_waiting_;
   pthread_cond_t *cond_customer_served_;
   pthread_cond_t *cond_barber_paid_;
   pthread_cond_t *cond_barber_sleeping_;


   //-------------------------- Init ---------------------------------------
   // Initializes the mutex and conditions used in the monitor
   // Postconditions: the conditions and mutex are initialized to default
   void init(int numCustomers, int numBarbers);


   //------------------------- int2string ---------------------------------------
   // Converts a given integer to a string
   // Postconditions: the conditions and mutex are initialized to default
   string int2string(int i);


   //------------------------- print ---------------------------------------
   // prints a message given a persons ID
   // Preconditions : Assumes negative valued IDs are barbers and positive
   //                 IDs represent customers
   // Postconditions: the message is printed to the console
   void print(int person, string message);
};
#endif
