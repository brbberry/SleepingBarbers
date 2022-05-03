#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop_org.h"
using namespace std;

void *barber(void *);
void *customer(void *);

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread. 
class ThreadParam
{
public:
    ThreadParam(Shop_org* shop, int id, int service_time) :
        shop(shop), id(id), service_time(service_time) {};
    Shop_org* shop;         
    int id;             
    int service_time;    
};

int main(int argc, char *argv[]) 
{

   // Read arguments from command line
   // TODO: Validate values
   if (argc != 5)
   {
       cout << "Usage: num_chairs num_customers service_time" << endl;
       return -1;
   }

    // number of waiting chairs
   int num_chairs       = atoi(argv[1]);
   // number of incoming customers
   int num_customers    = atoi(argv[2]);
   // number of barbers
   int num_barbers      = atoi(argv[3]);
   // service time
   int service_time     = atoi(argv[4]);

   //Single barber, one shop, many customers
   // will need to make this an array
   pthread_t barber_thread[num_barbers];
   pthread_t customer_threads[num_customers];
   // 0 for number of barbers
   // added num_customers 
   Shop_org shop(num_customers, num_chairs,0);
  
   ThreadParam* barber_param = new ThreadParam(&shop, 0, service_time);
   // 0 for the time being to preserve
   pthread_create(&barber_thread[0], NULL, barber, barber_param);

   for (int i = 0; i < num_customers; i++) 
   {
      usleep(rand() % 1000);
      int id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   // Wait for customers to finish and cancel barber
   for (int i = 0; i < num_customers; i++)
   {
       pthread_join(customer_threads[i], NULL);
   }
   // 0 for the time being as we are only working with 1
   pthread_cancel(barber_thread[0]);

   cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
   return 0;
}

void *barber(void *arg) 
{
   ThreadParam* barber_param = (ThreadParam*) arg;
   
   Shop_org& shop = *barber_param->shop;
   int service_time = barber_param->service_time;
   delete barber_param;

   while(true) 
   {
       //ADD ID barber isntead of 0
      shop.helloCustomer(0);
      usleep(service_time);
      //ADD ID of barber instead of 0
      shop.byeCustomer(0);
   }
   return nullptr;
}

void *customer(void *arg) 
{
   ThreadParam* customer_param = (ThreadParam*)arg;
   Shop_org& shop = *customer_param->shop;
   int id = customer_param->id;
   delete customer_param;

   if (shop.visitShop(id) == true)
   {
       shop.leaveShop(id);
   }
   return nullptr;
}
