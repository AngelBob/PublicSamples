// FakeGuard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define WINDOWS_LEAN_AND_MEAN

#include <windows.h>
#include <synchapi.h>

static const unsigned long KEEP_ALIVE_TIME = 5 * 30 * 1000; // <--- 5 minutes in milliseconds

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
    )
{
    HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, L"FakeGuardWait");
    if( NULL != hEvent )
    {
        // Do it the right way and wait for the event to signal...
        // Event never signals, so the wait hits the timeout interval.
        DWORD result = WaitForSingleObject( hEvent, KEEP_ALIVE_TIME );
        CloseHandle( hEvent );
    }
    else
    {
        // Brute force it with a Sleep()...
        Sleep( KEEP_ALIVE_TIME );
    }

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
