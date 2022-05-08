// Blake Berry
// 05/06/2022
// Homework 2
// This Program is an implimentation of the shop monitor. The monitor holds 
// a mutex protecting its general access along with condition variables that
// scale with the number of barbers. Various shared boolean values are 
// gaurded
//-----------------------------------------------------------------------------
#include "Shop.h"


//-------------------------- Init ---------------------------------------
// Initializes the mutex and conditions used in the monitor
// Postconditions: the conditions and mutex are initialized to default
void Shop::init(int numCustomers, int numBarbers)
{

   cond_customer_served_   = new pthread_cond_t[numBarbers + 1];
   cond_barber_paid_       = new pthread_cond_t[numBarbers + 1];
   cond_barber_sleeping_   = new pthread_cond_t[numBarbers + 1];

   for (int i = 0; i <= numBarbers; i++)
   {
      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }

   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_mutex_init(&mutex_, NULL);
}


//------------------------- int2string ---------------------------------------
// Converts a given integer to a string
// Postconditions: the conditions and mutex are initialized to default
string Shop::int2string(int i)
{
   stringstream out;
   out << i;
   return out.str();
}


//------------------------- print ---------------------------------------
// prints a message given a persons ID
// Preconditions : Assumes negative valued IDs are barbers and positive
//                 IDs represent customers
// Postconditions: the message is printed to the console
void Shop::print(int person, string message)
{
   bool customer = true;
   if (person < 0)
   {
      customer = false;
      person *= -1;
   }
   cout << ((customer) ? "customer[" : "barber  [") << person << "]: " <<
           message << endl;
}


//------------------------- get_cust_drops ---------------------------------------
// returns the number of customers dropped
// Postconditions: the number of customers not serviced is returned
int Shop::get_cust_drops() const
{
   return cust_drops_;
}


//-------------------------- Visit Shop ---------------------------------------
// A customer thread method that simulates a customer entering a barber shop.
// The customer thread will leave the shop if there are not enough waiting 
// chairs
// preconditions : assumes the conditionals have been instantiated
// Postconditions: prints the customer barber communication to the barber
//                 leaves the customer in the barbers service chair
//                 returns the ID of the barber cutting hair
int Shop::visitShop(int id)
{
   pthread_mutex_lock(&mutex_);

   // if there are no more waiting chairs leave
   if (waiting_chairs_.size() == max_waiting_cust_)
   {
      print(id, "leaves the shop because of no available waiting git chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   // if the lobby has filled waiting chairs or there is not a ready barber
   // wait in a chair
   if (!waiting_chairs_.empty() || barbers_ready_.empty())
   {
      print(id, "takes a waiting chair. # waiting seats available = " + 
            int2string(max_waiting_cust_ - waiting_chairs_.size()));
   }

   waiting_chairs_.push(id);

   // while there isnt a ready barber or I am not the next customer wait
   while (barbers_ready_.empty() || waiting_chairs_.front() != id)
   {
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
   }

   // leave the line
   waiting_chairs_.pop();

   // get the next barber
   int barberThatWillService = barbers_ready_.front();
   barbers_ready_.pop();

   print(id, "moves to the service chair " + int2string(barberThatWillService) +
         ". # waiting seats available = " + 
         int2string(max_waiting_cust_ - waiting_chairs_.size()));

   // sit in service chair
   customer_in_chair_[barberThatWillService]    = id;
   in_service_[barberThatWillService]           = true;

   pthread_cond_signal(&cond_barber_sleeping_[barberThatWillService]);
   pthread_mutex_unlock(&mutex_);

   return barberThatWillService;
}


//-------------------------- Leave Shop ---------------------------------------
// A customer thread method that simulates a customer leaving a barber shop.
// The customer thread pays the barber and then leaves the shop
// preconditions : assumes the conditionals have been instantiated
// Postconditions: signals to the barber it has paid and leaves the shop
void Shop::leaveShop(int id, int barberID)
{

   pthread_mutex_lock(&mutex_);

   print(id, "wait for barber[" + int2string(barberID) +
         "] to be done with hair-cut");

   // wait for the barber to finish cutting hair
   while (in_service_[barberID] == true)
   {
      pthread_cond_wait(&cond_customer_served_[barberID], &mutex_);
   }

   // pay the barber
   money_paid_[barberID] = true;
   pthread_cond_signal(&cond_barber_paid_[barberID]);

   // leave
   print(id, "says good-bye to the barber[" + int2string(barberID) + "].");
   pthread_mutex_unlock(&mutex_);
}


//-------------------------- Hello Customer -----------------------------------
// A barber thread method simulating the greeting of a customer for a haircut
// preconditions : assumes the conditionals have been instantiated
//                 assumes the customer enter shop method signals its arrival
// Postconditions: the barber is not waiting and is ready to cut hair
void Shop::helloCustomer(int id)
{

   pthread_mutex_lock(&mutex_);

   // check if the barber is ready to serve clients
   if (ready_to_serve[id] == false)
   {
      barbers_ready_.push(id);
   }

   // while there is no one in the lobby and mt chair is empty
   while (waiting_chairs_.empty() && customer_in_chair_[id] == 0)
   {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   // check if a customer sat down in my chair
   while (customer_in_chair_[id] == 0) // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   print(-id, "starts a hair-cut service for customer[" + 
         int2string(customer_in_chair_[id]) + "].");

   pthread_mutex_unlock(&mutex_);
}


//--------------------------Bye Customer -------------------------------------
// A barber thread method simulating the finishing of a customer for a haircut
// preconditions : assumes the conditionals have been instantiated
//                 assumes the customer leave shop method signals to the barber
//                 they have paid
// Postconditions: singals to the customer theyre being served and that the
//                 next customer can be serviced
void Shop::byeCustomer(int id)
{
   pthread_mutex_lock(&mutex_);

   // let the customer know they are done and need to pay
   in_service_[id] = false;
   print(-id, "says he's done with a hair-cut service for customer[" + 
               int2string(customer_in_chair_[id]) + "].");
   money_paid_[id] = false;
   pthread_cond_signal(&cond_customer_served_[id]);

   // wait to be paid
   while (money_paid_[id] == false)
   {
      pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
   }

   // mark the customer as gone, and get ready to serve
   // the next customer
   customer_in_chair_[id] = 0;
   print(-id, "calls in another customer");
   barbers_ready_.push(id);
   ready_to_serve[id] = true;
   pthread_cond_broadcast(&cond_customers_waiting_);

   pthread_mutex_unlock(&mutex_); // unlock
}