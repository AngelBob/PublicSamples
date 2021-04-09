// UniqueLockTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>

struct MustLock
{
public:
    inline auto GetLock()
    {
        return std::unique_lock<std::mutex>( m_Mutex );
    }

private:
    std::mutex m_Mutex;
};

void thread_proc( MustLock& lock, char threadName )
{
    for( int i = 0; i < 5; ++i )
    {
        // Sleep a bit to allow another thread to run
        using namespace std::chrono_literals;
        std::this_thread::sleep_for( 500ms );

        // Take the lock and block the other threads
        auto haveLock = lock.GetLock();
        std::cout << "Thread " << threadName << " has lock.  ";

        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for( 2000ms );
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "waited " << elapsed.count() << " ms.  Releasing lock." << std::endl;
    }
}

int main()
{
    MustLock locker;

    std::thread th1( thread_proc, std::ref( locker ), '1' );
    std::thread th2( thread_proc, std::ref( locker ), '2' );
    std::thread th3( thread_proc, std::ref( locker ), '3' );

    th1.join();
    th2.join();
    th3.join();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
