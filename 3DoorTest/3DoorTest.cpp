// 3DoorTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <random>

void GetChoices(
    std::random_device& rdev,
    const unsigned int number,
    int* results )
{
    // Choose random numbers between 1 and 3 - these represent the doors
    // either the "users" choices or the correct results
    std::default_random_engine engine( rdev() );
    std::mt19937 gen( rdev() ); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib( 1, 3 );

    for( unsigned int i = 0; i < number; ++i )
    {
        results[i] = ( distrib( gen ) ) - 1;
    }
}

int main( int argc, char* argv[] )
{
    // Seed with a real random value, if available
    std::random_device rdev;

    int count = atoi( argv[1] );

    std::cout << "Running scenarios with " << count << " iterations:\n";

    // Chose the "users" door
    int* choice = new int[ count ];
    GetChoices( rdev, count, choice );

    // Chose the "prize" door
    int* prize = new int[ count ];
    GetChoices( rdev, count, prize );

    int straight_wins = 0;
    int switch_wins = 0;
    for( int i = 0; i < count; ++i )
    {
        // Calculate win% when user does not change doors.
        // One case:
        // The user picks the winning door.
        if( choice[ i ] == prize[ i ] )
        {
            ++straight_wins;
        }

        // Calculate win% when user does change doors.
        // Two cases:
        // The user picks a losing door and must switch to the winner
        // because the game host has already eliminated the other loser, or
        // the user picks the winning door and must switch to a loser.
        // Just count the first case.
        // Clearly the percent chance is (1 - win%) from above, since we're
        // literally counting the opposite case.
        if( choice[ i ] != prize[ i ] )
        {
            ++switch_wins;
        }
    }

    float straight_win_pct = ( static_cast< float >( straight_wins ) / static_cast< float >( count ) ) * 100.0f;
    std::cout << "Win percent no change: " << straight_win_pct << "\n";

    float switch_win_pct = ( static_cast< float >( switch_wins ) / static_cast< float >( count ) ) * 100.0f;
    std::cout << "Win percent w/ change: " << switch_win_pct << "\n";

    std::cout << "Total win percentage: " << ( straight_win_pct + switch_win_pct ) << std::endl;

    delete[] choice;
    delete[] prize;
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
