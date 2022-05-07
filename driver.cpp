// Blake Berry
// 05/06/2022
// Homework 2
// This Program driver simulates the barber shop probelm. Barber threads and 
// customer threads are created and serviced using synchronization through
// a monitor. The customers are servied in a FCFS order
//-----------------------------------------------------------------------------

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
    ThreadParam(Shop_org *shop, int id, int service_time) : shop(shop), id(id), service_time(service_time){};
    Shop_org *shop;
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
    int num_barbers = atoi(argv[1]);
    // number of incoming customers
    int num_chairs = atoi(argv[2]);
    // number of barbers
    int num_customers = atoi(argv[3]);
    // service time
    int service_time = atoi(argv[4]);

    // condition checks
    if (num_barbers <= 0)
    {
        cout << "No barbers, The Shop Closes" << endl;
        return -1;
    }

    if (num_chairs < 0)
    {
        cout << "Cannot have a negative amount of waiting chairs" << endl;
        return -1;
    }

    if (num_customers < 0)
    {
        cout << "Cannot have a negative amount of customers" << endl;
        return -1;
    }

    if (service_time < 0)
    {
        cout << "Cannot have a negative service time" << endl;
        return -1;
    }

    // threads
    pthread_t barber_thread[num_barbers];
    pthread_t customer_threads[num_customers];

    Shop_org shop(num_customers, num_chairs, num_barbers);

    // babrber thread initailization
    for (int i = 0; i < num_barbers; i++)
    {
        int id = i + 1;
        ThreadParam *barber_param = new ThreadParam(&shop, id, service_time);
        pthread_create(&barber_thread[i], NULL, barber, barber_param);
    }

    // customer thread initialization
    for (int i = 0; i < num_customers; i++)
    {
        usleep(rand() % 1000);
        int id = i + 1;
        ThreadParam *customer_param = new ThreadParam(&shop, id, 0);
        pthread_create(&customer_threads[i], NULL, customer, customer_param);
    }

    // Wait for customers to finish and cancel barber
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    // end the barber threads
    for (int i = 0; i < num_barbers; i++)
    {
        pthread_cancel(barber_thread[i]);
    }

    cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
    return 0;
}

void *barber(void *arg)
{
    ThreadParam *barber_param = (ThreadParam *)arg;

    Shop_org &shop = *barber_param->shop;
    int service_time = barber_param->service_time;
    int id1 = barber_param->id;
    delete barber_param;

    while (true)
    {
        // ADD ID barber isntead of 0
        shop.helloCustomer(id1);
        usleep(service_time);
        // ADD ID of barber instead of 0
        shop.byeCustomer(id1);
    }
    return nullptr;
}

void *customer(void *arg)
{
    ThreadParam *customer_param = (ThreadParam *)arg;
    Shop_org &shop = *customer_param->shop;
    int id1 = customer_param->id;
    delete customer_param;

    // change to return int
    int barberID = shop.visitShop(id1);
    if (barberID > 0)
    {
        // cout << "\n testing barber ID for thread " << id1 <<" \t " << barberID << endl;
        shop.leaveShop(id1, barberID);
    }
    return nullptr;
}
