// Day04.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>

static bool read_input( void )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the lines and create the input data.

    // Step 3: return success or failure.
    return true;
}

int main()
{
    if( !read_input() )
    {
        return -1;
    }

    return 0;
}
