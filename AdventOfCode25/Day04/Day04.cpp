// Day04.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "puzzle_map.h"

static bool read_input( puzzle_map& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the lines and create the input data.
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        if( !input.add_row( line ) )
        {
            // Not all rows have the same number of entries.
            return false;
        }
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    puzzle_map input;
    if( !read_input( input ) )
    {
        return -1;
    }

    size_t accessible_rolls = input.count_accessible_rolls();
    std::cout << "There are " << accessible_rolls << " accessible rolls." << std::endl;

    return 0;
}
