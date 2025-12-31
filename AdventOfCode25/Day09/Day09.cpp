// Day09.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "vec2.hpp"

static vec2 split( const std::string& str )
{
    std::stringstream ss( str );
    std::string token;
    std::array<int64_t, 2> values{ 0, 0 };

    size_t idx = 0;
    while( std::getline( ss, token, ',' ) )
    {
        int64_t value = std::stoll( token );
        if( 0 != value )
        {
            values[ idx++ ] = value;
        }
    }

    return { values[ 0 ], values[ 1 ] };
}

static bool read_input(
    const std::string& filename,
    std::vector<vec2>& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( filename );

    // Step 2: read the lines and create the input data.
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        input.emplace_back( split( line ) );
    }

    // Step 3: return success or failure.
    return true;
}

static void calculate_areas(
    const std::vector<vec2>& input,
    area_map_t& areas
)
{
    for( uint16_t start = 0; start < input.size() - 1; ++start )
    {
        for( uint16_t end = start + 1; end < input.size(); ++end )
        {
            double area = input[ start ].area( input[ end ] );
            std::pair<uint16_t, uint16_t> nodes = std::make_pair( start, end );

            areas.emplace( std::make_pair( area, nodes ) );
        }
    }
}

int main()
{
#if 1
    static const std::string filename( ".\\Data\\Input_test.txt" );
#else
    static const std::string filename( ".\\Data\\Input.txt" );
#endif

    std::vector<vec2> input;
    if( !read_input( filename, input ) )
    {
        return -1;
    }

    area_map_t areas;
    calculate_areas( input, areas );
    std::cout << "The largest area is ";
    std::cout << static_cast<int64_t>( ( *areas.begin() ).first );
    std::cout << std::endl;
}