// Day02.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "range.h"

template<bool t>
static bool read_input( std::list<range<t>>& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the line and create the input data.
    do
    {
        uint64_t first = 0;
        uint64_t second = 0;
        char separator = '0';

        // Input is in the form "A-B", where A and B are integers.
        // Each pair is separated by a comma
        file >> first;
        file >> separator;
        file >> second;
        file >> separator;

        input.push_back( range<t>( first, second ) );

        if( file.eof() )
        {
            break;
        }
    } while( 1 );

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::list<range<false>> input1;
    std::list<range<true>> input2;
    if( !read_input( input1 ) ||
        !read_input( input2 ) )
    {
        return -1;
    }

    uint64_t sum = 0;
    for( const auto& test : input1 )
    {
        sum += test.total_invalid_entries();
    }
    std::cout << "Total of invalid entries: " << sum << std::endl;

    sum = 0;
    for( const auto& test : input2 )
    {
        sum += test.total_invalid_entries();
    }
    std::cout << "Total of invalid entries: " << sum << std::endl;

    return 0;
}
