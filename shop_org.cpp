
#include "Shop_org.h"

void Shop_org::init(int numCustomers, int numBarbers)
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

string Shop_org::int2string(int i)
{
   stringstream out;
   out << i;
   return out.str();
}

void Shop_org::print(int person, string message)
{
   bool customer = true;
   if (person < 0)
   {
      customer = false;
      person *= -1;
   }
   cout << ((customer) ? "customer[" : "barber  [") << person << "]: " << message << endl;
}

int Shop_org::get_cust_drops() const
{
   return cust_drops_;
}

int Shop_org::visitShop(int id)
{
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_)
   {
      print(id, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   waiting_chairs_.push(id);
   // if the ready barber q is empty customer_in_chair_[0] != 0
   if (barbers_ready_.empty() && !waiting_chairs_.empty())
   {
      // waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   }


   while (barbers_ready_.empty() || waiting_chairs_.front() != id)
   {
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
   }

   waiting_chairs_.pop();



   int barberThatWillService = barbers_ready_.front();
   barbers_ready_.pop();

   print(id, "moves to the service chair " + int2string(barberThatWillService) +". # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));


   customer_in_chair_[barberThatWillService]    = id;
   in_service_[barberThatWillService]           = true;
   
   pthread_cond_signal(&cond_barber_sleeping_[barberThatWillService]);

   pthread_mutex_unlock(&mutex_);
   return barberThatWillService;
}

void Shop_org::leaveShop(int id, int barberID)
{

   pthread_mutex_lock(&mutex_);

   print(id, "wait for barber["+ int2string(barberID) +"] to be done with hair-cut");

   while (in_service_[barberID] == true)
   {
      pthread_cond_wait(&cond_customer_served_[barberID], &mutex_);
   }

   money_paid_[barberID] = true;

   pthread_cond_signal(&cond_barber_paid_[barberID]);
   print(id, "says good-bye to the barber[" + int2string(barberID) +"].");
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::helloCustomer(int id)
{

   pthread_mutex_lock(&mutex_);

   barbers_ready_.push(id);
   while (waiting_chairs_.empty() && customer_in_chair_[id] == 0)
   {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   // check if a customer sat down
   while (customer_in_chair_[id] == 0) // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   print(-id, "starts a hair-cut service for customer[" + int2string(customer_in_chair_[id])+"].");
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::byeCustomer(int id)
{
   pthread_mutex_lock(&mutex_);

   in_service_[id] = false;
   print(-id, "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_[id])+"].");
   money_paid_[id] = false;

   pthread_cond_signal(&cond_customer_served_[id]);
   while (money_paid_[id] == false)
   {
      pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
   }


   customer_in_chair_[id] = 0;
   print(-id, "calls in another customer");

   pthread_cond_broadcast(&cond_customers_waiting_);
   pthread_mutex_unlock(&mutex_); // unlock
}
