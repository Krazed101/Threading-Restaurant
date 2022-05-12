/* 
 * Task #4 Specialties Threading-Restuarant
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
#define GRILL_TIME 4 * TIME_UNIT        //The total time needed for the grill.
#define FRYER_TIME 3 * TIME_UNIT        //The total time needed for the fries.
#define TOASTER_TIME 1 * TIME_UNIT      //The total time needed for the toaster.
#define MICROWAVE_TIME 2 * TIME_UNIT    //The total time needed for the microwave.
#define ASSEMBLY_TIME 1 * TIME_UNIT     //The total time needed for the assembly.

using namespace std;

void error(string err_msg);
void flex_worker();
void fry_worker();
void grill_worker();

//The class that defines what an order is.
class order
{
private:
public:
    unsigned int burgers_needed;        //The amount of burgers needed by this order.
    unsigned int buns_needed;           //The amount of buns needed by this order.
    unsigned int bacon_needed;          //The amount of bacon needed by this order.
    unsigned int fries_needed;          //The amount of fries needed by this order.

    //The constructor for this class.
    order(unsigned int burger_count, unsigned int bun_count, unsigned int bacon_count, unsigned int fry_count);
};

order::order(unsigned int burger_count, unsigned int bun_count, unsigned int bacon_count, unsigned int fry_count)
{
    //Constructor stuff happening here.
    burgers_needed = burger_count;
    buns_needed = bun_count;
    bacon_needed = bacon_count;
    fries_needed = fry_count;
}

//A queue that holds all the orders that need to be done.
queue<order> orders;

//The quantity needed of each component based on the current orders read in.
unsigned int burgers_needed = 0;
unsigned int fries_needed = 0;
unsigned int buns_needed = 0;
unsigned int bacon_needed = 0;

//The quantity made of each component that can used to assemble an order.
unsigned int burgers_ready = 0;
unsigned int fries_ready = 0;
unsigned int buns_ready = 0;
unsigned int bacon_ready = 0;

//Mutexs for the orders queue and appliances.
mutex orders_mutex;
mutex burgers;
mutex fries;
mutex toaster;
mutex microwave;
//One table to assemble on. And to help with everyone trying to assemble at once and pulling more than there actually is.
mutex assembly;

int main(int argc, char const *argv[])
{
    size_t num_grill_workers = 1;   //The number of grill workers threads. Default 1 just in case an error happens.
    size_t num_fry_workers = 1;     //The number of fry workers threads. Default 1 just in case an error happens.
    size_t num_flex_workers = 1;    //The number of flex workers threads. Default 1 just in case an error happens.
    queue<thread> threads;          //Queue of threads so that we can join them all in the end.
    srand(time(NULL));              //Seeding the rand function.

    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

    //Make sure we have the correct number of arguments or else let them know and return.
    if(argc != 4)
    {
        error("Incorrect Number of Arguments");
        cout << "Intended Usage:\n" << argv[0] << " <Number of Grill Workers> <Number of Fry Cooks> <Number of Flex Workers>\n";
        return 0;
    }
    cout << "Combo A: No burger just bones.\n";
    cout << "Combo B: Burger no bones here.\n";
    cout << "Combo C: Fries and burger plz.\n";
    cout << "\n";

    //Try to parse the user input to integers.
    try
    {
        num_grill_workers = stoi(string(argv[1]));
        num_fry_workers = stoi(string(argv[2]));
        num_flex_workers = stoi(string(argv[3]));

        if (num_grill_workers <= 0 || num_fry_workers <= 0 || num_flex_workers <= 0)
        {
            error("Incorrect Value of Arguments");
            cout << "Intended Usage:\n" << argv[0] << " <Number of Grill Workers> <Number of Fry Cooks> <Number of Flex Workers>\n";
            return 0;
        }
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

    //Our starving customers order 100 option A,B, or C's.
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
        //Randomly generate a combo A,B,or C.
        switch (rand() % 3 + 1)
        {
        case 1:
            //Create an order A of 2 burgers, 1 bun, 0 bacon, and 1 fry.
            orders.push(order(2,1,0,1));
            break;
        case 2:
            //Create an order A of 2 burgers, 2 bun, 1 bacon, and 1 fry.
            orders.push(order(2,2,1,1));
            break;
        case 3:
            //Create an order A of 1 burger, 1 bun, 0 bacon, and 0 fry.
            orders.push(order(1,1,0,0));
            break;
        }
    }

    cout << "\rCreating Order Queue for Testing Complete!\n";

    cout << "Creating Threads(Workers)...";

    auto start = chrono::high_resolution_clock::now();  //Start the timing.

    //Create the flex worker threads on the flex_worker function.
    for (size_t i = 0; i < num_flex_workers; i++)
    {
        threads.push(thread(flex_worker));
    }

    //Create the fry worker threads on the fry_worker function.
    for (size_t i = 0; i < num_fry_workers; i++)
    {
        threads.push(thread(fry_worker));
    }

    //Create the grill worker threads on the grill_worker function.
    for (size_t i = 0; i < num_grill_workers; i++)
    {
        threads.push(thread(grill_worker));
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
    cout << "Threads: " << num_flex_workers + num_grill_workers + num_fry_workers << "\n";
    
    return 0;
}

void flex_worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        orders_mutex.lock();                            //Lock the shared resource.
        order current_order = orders.front();           //Grab the current order from the queue.
        orders.pop();                                   //Pop the current order off the queue.
        orders_mutex.unlock();                          //Unlock the shared resource.
        burgers.lock();                                 //Lock the burger resource.
        burgers_needed += current_order.burgers_needed; //Add to the number needed.
        burgers.unlock();                               //Unlock the burger resource.
        fries.lock();                                   //Lock the fries resource.
        fries_needed += current_order.fries_needed;     //Add to the number needed.
        fries.unlock();                                 //Unlock the fries resource.
        toaster.lock();                                 //Lock the toaster resource.
        buns_needed += current_order.buns_needed;       //Add to the number needed.
        toaster.unlock();                               //Unlock the fries resource.
        microwave.lock();                               //Lock the microwave resource.
        bacon_needed += current_order.bacon_needed;     //Add to the number needed.
        microwave.unlock();                             //Unlock the microwave resource.
        
        //Keep making buns and bacon until there's enough and the assembly table is available.
        do
        {
            //If buns need to be made.
            if(buns_needed > 0) {
                //If the toaster is available.
                if(toaster.try_lock()){
                    this_thread::sleep_for(chrono::milliseconds(TOASTER_TIME));     //Sleep for alotted time for the toaster.
                    buns_needed -= 1;                                               //Remove from the needed amount.
                    buns_ready += 1;                                                //Add to the amount ready.
                    toaster.unlock();                                               //Unlock the toaster for others.
                }
            }
            //If bacon needs to be made.
            if(bacon_needed > 0) {
                //If the microwave is available.
                if(microwave.try_lock()){
                    this_thread::sleep_for(chrono::milliseconds(MICROWAVE_TIME));   //Sleep for alotted time for the microwave.
                    bacon_needed -= 1;                                              //Remove form the needed amount.
                    bacon_ready += 1;                                               //Add to the amount ready.
                    microwave.unlock();                                             //Unlock the microwave for others.
                }
            }
        } while (!assembly.try_lock() && 
                 burgers_ready < current_order.burgers_needed && 
                 fries_ready < current_order.fries_needed && 
                 buns_ready < current_order.buns_needed && 
                 bacon_ready < current_order.bacon_needed);

        burgers.lock();                                                 //Lock the burger resource.
        burgers_ready -= current_order.burgers_needed;                  //Remove how many burgers we need from the ready count.
        burgers.unlock();                                               //Unlock the burger resource.
        fries.lock();                                                   //Lock the fries resource.
        fries_ready -= current_order.fries_needed;                      //Remove how many fries we need from the ready count.
        fries.unlock();                                                 //Unlock the fries resource.
        toaster.lock();                                                 //Lock the toaster resource.
        buns_ready -= current_order.buns_needed;                        //Remove how many buns we need from the ready count.
        toaster.unlock();                                               //Unlock the fries resource.
        microwave.lock();                                               //Lock the microwave resource.
        bacon_ready -= current_order.bacon_needed;                      //Remove how many bacons we need from the ready count.
        microwave.unlock();                                             //Unlock the microwave resource.
        this_thread::sleep_for(chrono::milliseconds(ASSEMBLY_TIME));    //Sleep for the alotted assembly time.
        assembly.unlock();                                              //Unlock the assembly resource.
    }
}

void fry_worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        //If fries need to be made.
        if(fries_needed > 0)
        {
            this_thread::sleep_for(chrono::milliseconds(FRYER_TIME));   //Sleep for the alotted fry time.
            fries.lock();                                               //Lock the fries resource.
            fries_needed -= 2;                                          //Remove from the needed count.
            fries_ready += 2;                                           //Add to the ready count.
            fries.unlock();                                             //Unlock the fries resource.
        }
    }
}

void grill_worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        //If burgers need to be made.
        if(burgers_needed > 0)
        {
            this_thread::sleep_for(chrono::milliseconds(GRILL_TIME));   //Sleep for the alotted grill time.
            burgers.lock();                                             //Lock the frill resource.
            burgers_needed -= 4;                                        //Remove from the needed count.
            burgers_ready += 4;                                         //Add to the ready count.
            burgers.unlock();                                           //Unlock the grill resource.
        }
    }
}

void error(string err_msg) {
    //print the error in a standard error format.
    cout << "  ERROR:\t\t" << err_msg << "\n";
}