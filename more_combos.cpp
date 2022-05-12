/* 
 * Task #2 More Combos Threading-Restuarant
 * Author: Grant Ogden 
 */
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdlib.h>

#define ORDER_QTY 100       //The size of the test set.
#define TIME_UNIT 1000      //The time to complete one work order.

using namespace std;

void error(string err_msg);
void worker();

//A queue that holds all the orders that need to be done.
queue<char> orders;

//Mutexs for the orders queue and combos.
mutex orders_mutex;
mutex combo_a_mutex;
mutex combo_b_mutex;
mutex combo_c_mutex;

//Durations for the averaging of each combo
chrono::duration<double, milli> combo_a_time;
chrono::duration<double, milli> combo_b_time;
chrono::duration<double, milli> combo_c_time;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;     //The Number of threads. At least one just so everything gets done incase of error.
    queue<thread> threads;      //Queue of threads so that we can join them all in the end.
    int combo_a_count = 0;      //The count of how many combo A there are.
    int combo_b_count = 0;      //The count of how many combo B there are.
    int combo_c_count = 0;      //The count of how many combo C there are.
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
            combo_a_count++;
            break;
        case 2:
            orders.push('B');
            combo_b_count++;
            break;
        case 3:
            orders.push('C');
            combo_c_count++;
            break;
        }
    }
    cout << "\rCreating Order Queue for Testing Complete!\n";

    cout << "Creating Threads(Workers)...";

    auto start = chrono::high_resolution_clock::now();  //Start the timing.

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

    //Calculate the total time and averages then output it to the user.
    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_threads << "\n";
    cout << "Combo A Average Time: " << combo_a_time.count()/combo_a_count << "ms\n";
    cout << "Combo B Average Time: " << combo_b_time.count()/combo_b_count << "ms\n";
    cout << "Combo C Average Time: " << combo_c_time.count()/combo_c_count << "ms\n";
    
    return 0;
}

void worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        auto start = chrono::high_resolution_clock::now();  //Grab time for the average timing.
        orders_mutex.lock();            //Lock the shared resource.
        char order = orders.front();    //Grab the current order from the queue.
        orders.pop();                   //Pop the current order off the queue.
        orders_mutex.unlock();          //Unlock the shared resource.
        
        //Switch on the type of order.
        switch (order)
        {
        case 'A':
            this_thread::sleep_for(chrono::milliseconds(10 * TIME_UNIT));       //Sleep for the time needed to work.
            combo_a_mutex.lock();                                               //Lock for the shared resource.
            combo_a_time += (chrono::high_resolution_clock::now() - start);     //Add the time taken.
            combo_a_mutex.unlock();                                             //Unlock the shared resource.
            break;
        
        case 'B':
            this_thread::sleep_for(chrono::milliseconds(12 * TIME_UNIT));       //Sleep for the time needed to work.
            combo_b_mutex.lock();                                               //Lock for the shared resource.
            combo_b_time += (chrono::high_resolution_clock::now() - start);     //Add the time taken.
            combo_b_mutex.unlock();                                             //Unlock the shared resource.
            break;

        case 'C':
            this_thread::sleep_for(chrono::milliseconds(7 * TIME_UNIT));        //Sleep for the time needed to work.
            combo_c_mutex.lock();                                               //Lock for the shared resource.
            combo_c_time += (chrono::high_resolution_clock::now() - start);     //Add the time taken.
            combo_c_mutex.unlock();                                             //Unlock the shared resource.
            break;  
        }
    }
}

void error( string err_msg) {
    //print the error in a standard error format.
    cout << "  ERROR:\t\t" << err_msg << "\n";
}