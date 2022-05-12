/* 
 * Task #3 Appliances Threading-Restuarant
 * Author: Grant Ogden 
 */
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdlib.h>

#define ORDER_QTY 100                   //The size of the test set.
#define TIME_UNIT 1000                  //The time to complete one work order.
#define BURGER_MASK 1                   //The bit mask for burgers  (000x).
#define BUN_MASK 2                      //The bit mask for buns     (00x0).
#define FRIES_MASK 4                    //The bit mask for fries    (0x00).
#define BACON_MASK 8                    //The bit mask for bacon    (x000).
#define GRILL_TIME 4 * TIME_UNIT        //The total time needed for the grill.
#define FRYER_TIME 3 * TIME_UNIT        //The total time needed for the fries.
#define TOASTER_TIME 1 * TIME_UNIT      //The total time needed for the toaster.
#define MICROWAVE_TIME 2 * TIME_UNIT    //The total time needed for the microwave.
#define ASSEMBLY_TIME 2 * TIME_UNIT     //The total time needed for the assembly.

using namespace std;

void error(string err_msg);
void worker();
void make_order(unsigned int order_mask);

//A queue that holds all the orders that need to be done.
queue<char> orders;

//Mutexs for the orders queue and appliances.
mutex orders_mutex;
mutex grill;
mutex deep_fryer;
mutex toaster;
mutex microwave;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;     //The Number of threads. At least one just so everything gets done incase of error.
    queue<thread> threads;      //Queue of threads so that we can join them all in the end.
    srand(time(NULL));          //Seeding the rand function.

    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

    //Make sure we have the correct number of arguments or else let them know and return.
    if(argc != 2)
    {
        error("Incorrect Number of Arguments");
        cout << "Intended Usage:\n" << argv[0] << " <Number of Threads>\n";
        return 0;
    }
    cout << "Combo A: No burger just bones.\n";
    cout << "Combo B: Burger no bones here.\n";
    cout << "Combo C: Fries and burger plz.\n";
    cout << "\n";

    //Try to parse the user input to an integer
    try
    {
        num_threads = stoi(string(argv[1]));
    }
    catch( invalid_argument const& ex)
    {
        error("Invalid Argument:\t" + string(ex.what()) + "\n");
    }
    catch( out_of_range const& ex)
    {
        error("Out Of Range:\t" + string(ex.what()) + "\n");
    }
    
    cout << "Creating Order Queue for Testing...";

    //Our starving customers order 100 option A,B, or C's
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
        //Randomly generate a combo A,B,or C.
        switch (rand() % 3 + 1)
        {
        case 1:
            orders.push('A');
            break;
        case 2:
            orders.push('B');
            break;
        case 3:
            orders.push('C');
            break;
        }
    }
    cout << "\rCreating Order Queue for Testing Complete!\n";

    cout << "Creating Threads(Workers)...";

    auto start = chrono::high_resolution_clock::now(); //Start the timing.

    //Create all the threads using the worker function.
    for (size_t i = 0; i < num_threads; i++)
    {
        threads.push(thread(worker));
    }

    cout << "\rCreating Threads(Workers) Completed!\n";

    //Until all the threads have joined keep checking on them.
    while (!threads.empty())
    {
        threads.front().join();
        threads.pop();
    }

    auto end = chrono::high_resolution_clock::now();    //Grab the end time.

    //Make sure the workers actually did all the test data.
    if(!orders.empty())
    {
        cout << "Not All Orders Have Been Fullfilled\n";
    }

    //Calculate the total time then output it to the user.
    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_threads << "\n";
    
    return 0;
}

void worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        orders_mutex.lock();            //Lock the shared resource.
        char order = orders.front();    //Grab the current order from the queue.
        orders.pop();                   //Pop the current order off the queue.
        orders_mutex.unlock();          //Unlock the shared resource.
        
        //Switch on the type of order.
        switch (order)
        {
        case 'A':
            //The Combo A has burger, bun, and fries.
            make_order(BURGER_MASK | BUN_MASK | FRIES_MASK);
            break;
        
        case 'B':
            //The Combo B has burger, bun, bacon, and fries.
            make_order(BURGER_MASK | BUN_MASK | BACON_MASK | FRIES_MASK);
            break;

        case 'C':
            //The Combo C has burger and bun.
            make_order(BURGER_MASK | BUN_MASK);
            break;
        }
    }
}

void error(string err_msg) {
    //print the error in a standard error format.
    cout << "  ERROR:\t\t" << err_msg << "\n";
}

void make_order(unsigned int order_mask) {
    //Loop until we complete the order.
    do
    {
        //If we still need to make the burger.
        if(order_mask && BURGER_MASK) {
            //If the lock is available then lock.
            if(grill.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(GRILL_TIME));       //Sleep for the alotted work time.
                order_mask ^= BURGER_MASK;                                      //Flip the mask bit to show it's done.
                grill.unlock();                                                 //Unlock the grill for everyone else.
            }
        }
        //If we still need to make the bun.
        if(order_mask && BUN_MASK) {
            //If the lock is available then lock.
            if(toaster.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(TOASTER_TIME));     //Sleep for the alotted work time.
                order_mask ^= BUN_MASK;                                         //Flip the mask bit to show it's done.
                toaster.unlock();                                               //Unlock the grill for everyone else.
            }
        }
        //If we still need to make the fries.
        if(order_mask && FRIES_MASK) {
            //If the lock is available then lock.
            if(deep_fryer.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(FRYER_TIME));       //Sleep for the alotted work time.
                order_mask ^= FRIES_MASK;                                       //Flip the mask bit to show it's done.
                deep_fryer.unlock();                                            //Unlock the grill for everyone else.
            }
        }
        //If we still need to make the bacon.
        if(order_mask && BACON_MASK) {
            //If the lock is available then lock.
            if(microwave.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(MICROWAVE_TIME));   //Sleep for the alotted work time.
                order_mask ^= BACON_MASK;                                       //Flip the mask bit to show it's done.
                microwave.unlock();                                             //Unlock the grill for everyone else.
            }
        }
    } while (order_mask != 0);
    
    //Sleep for the alotted assembly time.
    this_thread::sleep_for(chrono::milliseconds(ASSEMBLY_TIME));
    
    return;
}