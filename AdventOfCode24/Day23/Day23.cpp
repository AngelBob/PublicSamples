// Day23.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <string>

static bool read_input(
    std::multimap<std::string, std::string>& lan_pairs
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Lan.txt");

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    while( std::getline( file, line ) )
    {
        std::string first( { line[ 0 ], line[ 1 ] } );
        std::string second( { line[ 3 ], line[ 4 ] } );

        lan_pairs.insert( std::make_pair( first, second ) );
        lan_pairs.insert( std::make_pair( second, first ) );
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::multimap<std::string, std::string> lan_pairs;
    if( !read_input( lan_pairs ) )
    {
        return -1;
    }
}
