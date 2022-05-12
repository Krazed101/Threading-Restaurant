/* 
 * Task #1 Bare Bones Threading-Restuarant
 * Author: Grant Ogden 
 */
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#define ORDER_QTY 100       //The size of the test set.
#define WORK_TIME 10000     //The time to complete one work order.
using namespace std;

void error(string err_msg);
void worker();

//A queue that holds all the orders that need to be done.
queue<char> orders;

//Mutex for the orders queue so multiple threads don't pop at the same time.
mutex orders_mutex;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;     //The Number of threads. At least one just so everything gets done incase of error.
    queue<thread> threads;      //Queue of threads so that we can join them all in the end.

    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

    //Make sure we have the correct number of arguments or else let them know and return.
    if(argc != 2)
    {
        error("Incorrect Number of Arguments");
        cout << "Intended Usage:\n" << argv[0] << " <Number of Threads>\n";
        return 0;
    }
    cout << "Combo A: No burger just bones. All we got\n";
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
    
    

    //Our starving customers order 100 option A's.
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
        orders.push('A');
    }

    auto start = chrono::high_resolution_clock::now();  //Start the timing.

    //Create all the threads using the worker function.
    for (size_t i = 0; i < num_threads; i++)
    {
        threads.push(thread(worker));
    }

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
        cout << "Not All Orders Have Been Fullfilled";
    }

    //Calculate the total time and output it to the user.
    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_threads << "\n";
    
    return 0;
}

void worker() {
    //Loop while there are still orders to do.
    while (!orders.empty())
    {
        orders_mutex.lock();    //Lock the shared resource.
        orders.pop();           //Pop the current order off the queue.
        orders_mutex.unlock();  //Unlock the shared resource.
        this_thread::sleep_for(chrono::milliseconds(WORK_TIME));    //Do the work for the alloted time.
    }
}

void error( string err_msg) {
    //print the error in a standard error format.
    cout << "  ERROR:\t\t" << err_msg << "\n";
}