#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdlib.h>

#define ORDER_QTY 100
#define TIME_UNIT 1000
#define BURGER_MASK 1
#define BUN_MASK 2
#define FRIES_MASK 4
#define BACON_MASK 8
#define GRILL_TIME 4 * TIME_UNIT
#define FRYER_TIME 3 * TIME_UNIT
#define TOASTER_TIME 1 * TIME_UNIT
#define MICROWAVE_TIME 2 * TIME_UNIT
#define ASSEMBLY_TIME 2 * TIME_UNIT

using namespace std;

void error(string err_msg);
void worker();
void make_order(unsigned int order_mask);

queue<char> orders;
mutex orders_mutex;
mutex grill;
mutex deep_fryer;
mutex toaster;
mutex microwave;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;
    queue<thread> threads;

    srand(time(NULL));
    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

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
    //Our starving customers order 100 option A's
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
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
    auto start = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_threads; i++)
    {
        threads.push(thread(worker));
        //cout << "Thread Created!\n";
    }
    cout << "\rCreating Threads(Workers) Completed!\n";

    while (!threads.empty())
    {
        threads.front().join();
        threads.pop();
        //cout << "Thread Joined!\n";
    }
    auto end = chrono::high_resolution_clock::now();

    if(!orders.empty())
    {
        cout << "Not All Orders Have Been Fullfilled\n";
    }

    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_threads << "\n";
    
    return 0;
}

void worker() {
    while (!orders.empty())
    {
        auto start = chrono::high_resolution_clock::now();
        orders_mutex.lock();
        char order = orders.front();
        orders.pop();
        orders_mutex.unlock();
        
        switch (order)
        {
        case 'A':
            make_order(BURGER_MASK | BUN_MASK | FRIES_MASK);
            break;
        
        case 'B':
            make_order(BURGER_MASK | BUN_MASK | BACON_MASK | FRIES_MASK);
            break;

        case 'C':
            make_order(BURGER_MASK | BUN_MASK);
            break;
        }
    }
}

void error(string err_msg) {
    cout << "  ERROR:\t\t" << err_msg << "\n";
}

void make_order(unsigned int order_mask) {
    do
    {
        if(order_mask && BURGER_MASK) {
            if(grill.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(GRILL_TIME));
                order_mask ^= BURGER_MASK;
                grill.unlock();
            }
        }
        if(order_mask && BUN_MASK) {
            if(toaster.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(TOASTER_TIME));
                order_mask ^= BUN_MASK;
                toaster.unlock();
            }
        }
        if(order_mask && FRIES_MASK) {
            if(deep_fryer.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(FRYER_TIME));
                order_mask ^= FRIES_MASK;
                deep_fryer.unlock();
            }
        }
        if(order_mask && BACON_MASK) {
            if(microwave.try_lock()){
                this_thread::sleep_for(chrono::milliseconds(MICROWAVE_TIME));
                order_mask ^= BACON_MASK;
                microwave.unlock();
            }
        }
    } while (order_mask != 0);
    
    this_thread::sleep_for(chrono::milliseconds(ASSEMBLY_TIME));
    cout << "Order Complete!" << endl;
    return;
}