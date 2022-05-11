#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdlib.h>

#define APP_NAME "main.exe"
#define ORDER_QTY 100
#define TIME_UNIT 1000
using namespace std;

void error(string err_msg);
void worker();

queue<char> orders;
mutex orders_mutex;
mutex combo_a_mutex;
mutex combo_b_mutex;
mutex combo_c_mutex;
chrono::duration<double, milli> combo_a_time;
chrono::duration<double, milli> combo_b_time;
chrono::duration<double, milli> combo_c_time;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;
    queue<thread> threads;
    int combo_a_count = 0;
    int combo_b_count = 0;
    int combo_c_count = 0;
    srand(time(NULL));
    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

    if(argc != 2)
    {
        error("Incorrect Number of Arguments");
        cout << "Intended Usage:\n" << APP_NAME << " <Number of Threads>\n";
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
    cout << "Combo A Average Time: " << combo_a_time.count()/combo_a_count << "ms\n";
    cout << "Combo B Average Time: " << combo_b_time.count()/combo_b_count << "ms\n";
    cout << "Combo C Average Time: " << combo_c_time.count()/combo_c_count << "ms\n";
    
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
            this_thread::sleep_for(chrono::milliseconds(10 * TIME_UNIT));
            combo_a_mutex.lock();
            combo_a_time += (chrono::high_resolution_clock::now() - start);
            combo_a_mutex.unlock();
            break;
        
        case 'B':
            this_thread::sleep_for(chrono::milliseconds(12 * TIME_UNIT));
            combo_b_mutex.lock();
            combo_b_time += (chrono::high_resolution_clock::now() - start);
            combo_b_mutex.unlock();
            break;

        case 'C':
            this_thread::sleep_for(chrono::milliseconds(7 * TIME_UNIT));
            combo_c_mutex.lock();
            combo_c_time += (chrono::high_resolution_clock::now() - start);
            combo_c_mutex.unlock();
            break;
        }
    }
}

void error( string err_msg) {
    cout << "  ERROR:\t\t" << err_msg << "\n";
}