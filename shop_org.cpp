
#include "Shop_org.h"

void Shop_org::init(int numCustomers, int numBarbers)
{
   // each of these need a signal themselves becuase the barber
   // need to rely on this
   // +1 and <= so we can use ID's as indexes however the 0th will go unused
   cond_customer_served_ = new pthread_cond_t[numBarbers+1];
   cond_barber_paid_ = new pthread_cond_t[numBarbers+1];
   cond_barber_sleeping_ = new pthread_cond_t[numBarbers+1];

   for (int i = 0; i <= numBarbers; i++)
   {
      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
   // This way it will allow us to signal the appropriate flag

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
   if(person < 0) {
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

   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service

   // made this a while looop
   // while there is not the case customer in the barbers chair
   // or the waiting room is empty
   // maybe add front == id
   waiting_chairs_.push(id);
   // if the ready barber q is empty customer_in_chair_[0] != 0
   if (barbers_ready_.empty() && !waiting_chairs_.empty())
   {
      // waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));

      // using id as this this the signal that will need to be executed to the process for it to start
      // pthread_cond_wait(&cond_customers_waiting_[id], &mutex_);
      // moving this out of the while loop? Although I am kinda sure we can promise the front will be popped?
      // waiting_chairs_.pop();
   }

   // above we effectivly sat down and are waiting to be called. Now we really gotta wait until we are called
   // customer_in_chair_[0] != 0
   //barbers_ready_.empty() || 
   // || waiting_chairs_.front() != id
   while (barbers_ready_.empty() || waiting_chairs_.front() != id)
   {
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
   }


   waiting_chairs_.pop();

   //waiting_chairs_.pop();

   print(id, "moves to the service chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));

   // moving to hello barber to prepare the waiting loop so that it is signaled
   //////////////////////////////////////////////////////////////////////////////////////
   // currently using 0 as there is only 1 barber
   int barberThatWillService = barbers_ready_.front();
   barbers_ready_.pop();
   customer_in_chair_[barberThatWillService] = id;
   in_service_[barberThatWillService] = true;

   // wake up the barber just in case if he is sleeping
   // 0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_barber_sleeping_[barberThatWillService]);

   pthread_mutex_unlock(&mutex_);
   return barberThatWillService;
}

void Shop_org::leaveShop(int id, int barberID)
{
   // ADDED barberID instead of 0
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(id, "wait for the hair-cut to be done");

   // currently using 0 as there is only 1 barber
   while (in_service_[barberID] == true)
   {
      // 0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_customer_served_[barberID], &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_[barberID] = true;
   // 0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_barber_paid_[barberID]);
   print(id, "says good-bye to the barber.");
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::helloCustomer(int id)
{

   // overall I added id instead of index 0 for this function
   //  i also added a pop off the top
   pthread_mutex_lock(&mutex_);

   // If no customers than barber can sleep
   // currently using 0 as there is only 1 barber
   // changed customer_in_chair[id] == 0
   barbers_ready_.push(id);
   while (waiting_chairs_.empty() && customer_in_chair_[id] == 0)
   {
      print(-id, "sleeps because of no customers.");
      // 0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   // check if a customer sat down
   while (customer_in_chair_[id] == 0) // check if the customer, sit down.
   {
      // 0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_sleeping_[id], &mutex_);
   }

   //barbers_ready_.pop();

   // currently using 0 as there is only 1 barber
   print(-id, "starts a hair-cut service for " + int2string(customer_in_chair_[id]));
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::byeCustomer(int id)
{
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   // currently using 0 as there is only 1 barber
   in_service_[id] = false;
   // currently using 0 as there is only 1 barber
   print(-id, "says he's done with a hair-cut service for " + int2string(customer_in_chair_[id]));
   money_paid_[id] = false;
   // 0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_customer_served_[id]);
   while (money_paid_[id] == false)
   {
      // 0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_paid_[id], &mutex_);
   }

   // Signal to customer to get next one
   customer_in_chair_[id] = 0;
   print(-id, "calls in another customer");

   pthread_cond_broadcast(&cond_customers_waiting_);
   pthread_mutex_unlock(&mutex_); // unlock
}
