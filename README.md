## The Classic Barber Shop Problem
A barbershop consists of a waiting room with n waiting chairs one barber and one chair. When there no customers require a haircut, the barber takes a nap. If a customer enters the barbershop and the waiting room with n chairs is full, then the customer leaves the shop. If the barber is working with a customer and there are waiting chairs are available, then the customer takes one of the waiting chairs. If the barber is sleeping, the customer wakes up the barber and undergoes the haircut process.

## This Projects Extension
*	This barber shop will consist of n waiting chairs, x barbers, and y barber chairs. If there are no customers in the shop, then all the barbers are asleep.
*	Customers will receive a haircut from the barber and will be responsible for completing a payment transaction with the barber prior to their service being completed.
*	When the customer leaves it will signal that a spot has opened. The customers will be serviced in the order in which they enter the shop




## Program Flow
The barber threads get created and call the hello customer method. This method puts the barbers in a wait Q and go to sleep until the customers arrive.

The customer threads also get created and enters the shop by acquiring the mutex (currently the barbers are sleeping). The customer thread will check to see if the waiting room is full – if it is, then the customer thread will leave the shop and give up the mutex. The customer thread will then check to see if there is a ready barber or if there are already customers queued up in the waiting room – if this is the case it’ll take a chair in the barber shop, then while it waits to be at the front of the line and for the barber to be ready to serve the client.

When the barber is ready, the customer thread will move to that barber’s chair and wake it up for the service. After it performs this task, it unlocks. When the barber thread gets the signal that the hair cut is ready, it starts the haircut and unlocks. The haircut takes the given service time

While the haircut is occurring the customer thread waits for the barber to be done with cutting their hair, doing this it waits on the cond customer served[barber id] and unlocks. The barber then can acquire the lock and signal to the customer their hair cut is done. The barber will then wait for the customer to pay on a condition and unlock. The customer then pays the barber and signals to the barber they have been paid and leave the shop. The barber then prepares for the next service and signals to all the waiting customers that they are ready to serve the next in line.

These events can occur in any order and have been coordinated through conditional waiting to allow for concurrent barber shop operation. An illustration is present on the next page showing the flow between 1 barber thread and 1 customer thread. The red indicates condition waiting, the blue are signals sent from the customer to the barber, and the green indicates signals sent from the barber to the customer thread.
