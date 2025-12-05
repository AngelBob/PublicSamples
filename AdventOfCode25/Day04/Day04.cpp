// Day04.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "puzzle_map.h"

template<bool t>
static bool read_input( puzzle_map<t>& input )
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
    puzzle_map<false> input1;
    puzzle_map<true> input2;
    if( !read_input( input1 ) ||
        !read_input( input2 ) )
    {
        return -1;
    }

    size_t accessible_rolls = input1.count_accessible_rolls();
    std::cout << "There are " << accessible_rolls << " accessible rolls." << std::endl;

    accessible_rolls = input2.count_accessible_rolls();
    std::cout << "There are " << accessible_rolls << " accessible rolls with recursion." << std::endl;

    return 0;
}
