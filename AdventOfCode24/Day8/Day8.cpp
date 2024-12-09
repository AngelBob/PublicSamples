// Day8.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static bool read_input(
    std::multimap<char, std::pair<size_t, size_t>>& antenna_locations,
    size_t& extent // Makes a square grid.
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2: read each line and insert the strings into the grid.
    size_t y = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        size_t x = 0;
        for( auto& letter : line )
        {
            if( '.' != letter )
            {
                antenna_locations.emplace( letter, std::make_pair( x, y ) );
            }
            ++x;
        }
        ++y;
    }
    extent = y;

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::multimap<char, std::pair<size_t, size_t>> antenna_locations;
    size_t max_idx;
    if( !read_input( antenna_locations, max_idx ) )
    {
        return -1;
    }
}
