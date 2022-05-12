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
#define ASSEMBLY_TIME 1 * TIME_UNIT

using namespace std;

void error(string err_msg);
void flex_worker();
void fry_worker();
void grill_worker();

class order
{
private:
public:
    unsigned int burgers_needed;
    unsigned int buns_needed;
    unsigned int bacon_needed;
    unsigned int fries_needed;
    order(unsigned int burger_count, unsigned int bun_count, unsigned int bacon_count, unsigned int fry_count);
};

order::order(unsigned int burger_count, unsigned int bun_count, unsigned int bacon_count, unsigned int fry_count)
{
    burgers_needed = burger_count;
    buns_needed = bun_count;
    bacon_needed = bacon_count;
    fries_needed = fry_count;
}

queue<order> orders;
unsigned int burgers_needed = 0;
unsigned int fries_needed = 0;
unsigned int buns_needed = 0;
unsigned int bacon_needed = 0;

unsigned int burgers_ready = 0;
unsigned int fries_ready = 0;
unsigned int buns_ready = 0;
unsigned int bacon_ready = 0;


mutex orders_mutex;
mutex burgers;
mutex fries;
mutex toaster;
mutex microwave;
//One table to assemble on. And to help with everyone trying to assemble at once and pulling more than there actually is.
mutex assembly;

int main(int argc, char const *argv[])
{
    size_t num_grill_workers;
    size_t num_fry_workers;
    size_t num_flex_workers;
    queue<thread> threads;

    srand(time(NULL));
    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

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
    //Our starving customers order 100 option A's
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
        switch (rand() % 3 + 1)
        {
        case 1:
            orders.push(order(2,1,0,1));
            break;
        case 2:
            orders.push(order(2,2,1,1));
            break;
        case 3:
            orders.push(order(1,1,0,0));
            break;
        }
    }
    cout << "\rCreating Order Queue for Testing Complete!\n";

    cout << "Creating Threads(Workers)...";
    auto start = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_flex_workers; i++)
    {
        threads.push(thread(flex_worker));
    }
    for (size_t i = 0; i < num_fry_workers; i++)
    {
        threads.push(thread(fry_worker));
    }
    for (size_t i = 0; i < num_grill_workers; i++)
    {
        threads.push(thread(grill_worker));
    }
    cout << "\rCreating Threads(Workers) Completed!\n";

    while (!threads.empty())
    {
        threads.front().join();
        threads.pop();
    }
    auto end = chrono::high_resolution_clock::now();

    if(!orders.empty())
    {
        cout << "Not All Orders Have Been Fullfilled\n";
    }

    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_flex_workers + num_grill_workers + num_fry_workers << "\n";
    
    return 0;
}

void flex_worker() {
    while (!orders.empty())
    {
        orders_mutex.lock();
        order current_order = orders.front();
        orders.pop();
        orders_mutex.unlock();
        burgers.lock();
        burgers_needed += current_order.burgers_needed;
        burgers.unlock();
        fries.lock();
        fries_needed += current_order.fries_needed;
        fries.unlock();
        toaster.lock();
        buns_needed += current_order.buns_needed;
        toaster.unlock();
        microwave.lock();
        bacon_needed += current_order.bacon_needed;
        microwave.unlock();
        
        do
        {
            if(buns_needed > 0) {
                if(toaster.try_lock()){
                    this_thread::sleep_for(chrono::milliseconds(TOASTER_TIME));
                    buns_needed -= 1;
                    buns_ready += 1;
                    toaster.unlock();
                }
            }
            if(bacon_needed > 0) {
                if(microwave.try_lock()){
                    this_thread::sleep_for(chrono::milliseconds(MICROWAVE_TIME));
                    bacon_needed -= 1;
                    bacon_ready += 1;
                    microwave.unlock();
                }
            }
        } while (!assembly.try_lock() && 
                 burgers_ready < current_order.burgers_needed && 
                 fries_ready < current_order.fries_needed && 
                 buns_ready < current_order.buns_needed && 
                 bacon_ready < current_order.bacon_needed);

        burgers.lock();
        burgers_ready -= current_order.burgers_needed;
        burgers.unlock();
        fries.lock();
        fries_ready -= current_order.fries_needed;
        fries.unlock();
        toaster.lock();
        buns_ready -= current_order.buns_needed;
        toaster.unlock();
        microwave.lock();
        bacon_ready -= current_order.bacon_needed;
        microwave.unlock();
        this_thread::sleep_for(chrono::milliseconds(ASSEMBLY_TIME));
        assembly.unlock();
    }
}

void fry_worker() {
    while (!orders.empty())
    {
        if(fries_needed > 0)
        {
            this_thread::sleep_for(chrono::milliseconds(FRYER_TIME));
            fries.lock();
            fries_needed -= 2;
            fries_ready += 2;
            fries.unlock();
        }
    }
}

void grill_worker() {
    while (!orders.empty())
    {
        if(burgers_needed > 0)
        {
            this_thread::sleep_for(chrono::milliseconds(GRILL_TIME));
            burgers.lock();
            burgers_needed -= 4;
            burgers_ready += 4;
            burgers.unlock();
        }
    }
}

void error(string err_msg) {
    cout << "  ERROR:\t\t" << err_msg << "\n";
}