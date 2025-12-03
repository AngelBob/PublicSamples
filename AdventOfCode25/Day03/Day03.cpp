// Day03.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bank.h"

static bool read_input( std::vector<bank>& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the line and create the input data.
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );
        input.emplace_back( line );
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::vector<bank> banks;
    if( !read_input( banks ) )
    {
        return -1;
    }

    uint64_t sum = 0;
    for( const auto& bank : banks )
    {
        sum += bank.calculate_joltage( 2 );
    }
    std::cout << "The total joltage is: " << sum << std::endl;
}
