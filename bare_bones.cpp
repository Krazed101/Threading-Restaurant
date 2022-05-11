#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#define APP_NAME "main.exe"
#define ORDER_QTY 100
#define WORK_TIME 10000
using namespace std;

void error(string err_msg);
void worker();

queue<char> orders;

mutex orders_mutex;

int main(int argc, char const *argv[])
{
    size_t num_threads = 1;
    queue<thread> threads;
    cout << "Welcome to Ogden's Bare Bones Burger Shop\n";
    cout << "-----------------------------------------\n";

    if(argc != 2)
    {
        error("Incorrect Number of Arguments");
        cout << "Intended Usage:\n" << APP_NAME << " <Number of Threads>\n";
        return 0;
    }
    cout << "Combo A: No burger just bones. All we got\n";
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
    
    

    //Our starving customers order 100 option A's
    for (size_t i = 0; i < ORDER_QTY; i++)
    {
        orders.push('A');
    }

    auto start = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_threads; i++)
    {
        threads.push(thread(worker));
        //cout << "Thread Created!\n";
    }

    while (!threads.empty())
    {
        threads.front().join();
        threads.pop();
        //cout << "Thread Joined!\n";
    }
    auto end = chrono::high_resolution_clock::now();

    if(!orders.empty())
    {
        cout << "Not All Orders Have Been Fullfilled";
    }

    chrono::duration<double, milli> elapsed = end-start;
    cout << "Time Elapsed: " << elapsed.count() << "ms\n";
    cout << "Threads: " << num_threads << "\n";
    
    return 0;
}

void worker() {
    while (!orders.empty())
    {
        //orders.front(); Get the next order
        //int order_num = ORDER_QTY - orders.size() + 1;
        orders_mutex.lock();
        orders.pop();
        orders_mutex.unlock();
        this_thread::sleep_for(chrono::milliseconds(WORK_TIME));
        //cout << "Order " << order_num << " Completed!\n";
    }
}

void error( string err_msg) {
    cout << "  ERROR:\t\t" << err_msg << "\n";
}