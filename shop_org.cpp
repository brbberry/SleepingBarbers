
#include "Shop_org.h"

void Shop_org::init(int numCustomers, int numBarbers)
{
   // each of these need a signal themselves becuase the barber
   // need to rely on this
   cond_customer_served_ = new pthread_cond_t[numBarbers];
   cond_barber_paid_ = new pthread_cond_t[numBarbers];
   cond_barber_sleeping_ = new pthread_cond_t[numBarbers];
   cond_customers_waiting_ = new pthread_cond_t[numCustomers];

   for (int i = 0; i < numBarbers; i++)
   {
      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
   // This way it will allow us to signal the appropriate flag
   for(int i = 0; i < numCustomers; i++) {
      pthread_cond_init(&cond_customers_waiting_[i], NULL);
   }

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
   cout << ((person != barber) ? "customer[" : "barber  [") << person << "]: " << message << endl;
}

int Shop_org::get_cust_drops() const
{
   return cust_drops_;
}

bool Shop_org::visitShop(int id)
{
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_)
   {
      print(id, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return false;
   }

   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service

   // made this a while looop
   // while there is not the case customer in the barbers chair
   // or the waiting room is empty
   // maybe add front == id
   waiting_chairs_.push(id);
   if(customer_in_chair_[0] != 0 || !waiting_chairs_.empty())
   {
      //waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));

      // using id as this this the signal that will need to be executed to the process for it to start
      //pthread_cond_wait(&cond_customers_waiting_[id], &mutex_);
      // moving this out of the while loop? Although I am kinda sure we can promise the front will be popped?
      //waiting_chairs_.pop();
   }

   // above we effectivly sat down and are waiting to be called. Now we really gotta wait until we are called
   while(customer_in_chair_[0] != 0 || waiting_chairs_.front() != id) {
      pthread_cond_wait(&cond_customers_waiting_[id], &mutex_);
   }

   waiting_chairs_.pop();

   print(id, "moves to the service chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   // currently using 0 as there is only 1 barber
   customer_in_chair_[0] = id;
   in_service_[0] = true;

   // wake up the barber just in case if he is sleeping
   //0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_barber_sleeping_[0]);

   pthread_mutex_unlock(&mutex_);
   return true;
}

void Shop_org::leaveShop(int id)
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(id, "wait for the hair-cut to be done");

   // currently using 0 as there is only 1 barber
   while (in_service_[0] == true)
   {
      //0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_customer_served_[0], &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_[0] = true;
   //0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_barber_paid_[0]);
   print(id, "says good-bye to the barber.");
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::helloCustomer(int id)
{
   pthread_mutex_lock(&mutex_);

   // If no customers than barber can sleep
   // currently using 0 as there is only 1 barber
   while (waiting_chairs_.empty() && customer_in_chair_[0] == 0)
   {
      print(barber, "sleeps because of no customers.");
            //0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_sleeping_[0], &mutex_);
   }
   
   // check if a customer sat down
   while (customer_in_chair_[0] == 0) // check if the customer, sit down.
   {
            //0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_sleeping_[0], &mutex_);
   }
   
   // currently using 0 as there is only 1 barber
   print(barber, "starts a hair-cut service for " + int2string(customer_in_chair_[0]));
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::byeCustomer(int id)
{
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   // currently using 0 as there is only 1 barber
   in_service_[0] = false;
   // currently using 0 as there is only 1 barber
   print(barber, "says he's done with a hair-cut service for " + int2string(customer_in_chair_[0]));
   money_paid_[0] = false;
         //0 replace with barber ID to accomidate more
   pthread_cond_signal(&cond_customer_served_[0]);
   while (money_paid_[0] == false)
   {
            //0 replace with barber ID to accomidate more
      pthread_cond_wait(&cond_barber_paid_[0], &mutex_);
   }

   // Signal to customer to get next one
   customer_in_chair_[0] = 0;
   print(barber, "calls in another customer");

   // this is all new -- getting the waiting customer and signalling that thread
   if(!waiting_chairs_.empty()) {
      int frontOfLine = waiting_chairs_.front();
      pthread_cond_signal(&cond_customers_waiting_[frontOfLine]);
   }

   pthread_mutex_unlock(&mutex_); // unlock
}
