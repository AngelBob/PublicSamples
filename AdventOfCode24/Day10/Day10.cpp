// Day10.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_input(
    std::vector<std::string>& topo_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\TopoMap.txt" );

    // Step 2: read the single line of input.
    std::string line;
    while( std::getline( file, line ) )
    {
        topo_map.emplace_back( line );
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::vector<std::string> topo_map;
    if( !read_input( topo_map ) )
    {
        return -1;
    }
}
