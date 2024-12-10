// Day10.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_input(
    std::vector<std::vector<int32_t>>& topo_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\TopoMap.txt" );

    // Step 2: read the single line of input.
    std::string line;
    while( std::getline( file, line ) )
    {
        std::vector<int32_t> topo;
        topo.reserve( line.length() );

        for( size_t i = 0; i < line.length(); ++i )
        {
            const char digit = line[ i ];
            topo.emplace_back( digit - '0' );
        }
        topo_map.emplace_back( topo );
    }

    // Step 3: return success or failure.
    return true;
}

static int32_t find_path_step(
    std::vector<std::vector<int32_t>>& topo_map,
    const size_t start_row,
    const size_t start_col,
    const bool calc_rating
)
{
    // Is the trail at the top?
    if( 9 == topo_map[ start_row ][ start_col ] )
    {
        if( !calc_rating )
        {
            // Don't count this one again...
            topo_map[ start_row ][ start_col ] = -1;
        }

        // Add this to the reachable peaks count
        return 1;
    }

    // Try stepping north
    int32_t sum_scores = 0;
    if( 0 < start_row )
    {
        if( topo_map[ start_row - 1 ][ start_col ] ==
            ( topo_map[ start_row ][ start_col ] + 1 ) )
        {
            sum_scores += find_path_step( topo_map, start_row - 1, start_col, calc_rating );
        }
    }

    // Try stepping south
    if( start_row < topo_map.size() - 1 )
    {
        if( topo_map[ start_row + 1 ][ start_col ] ==
            ( topo_map[ start_row ][ start_col ] + 1 ) )
        {
            sum_scores += find_path_step( topo_map, start_row + 1, start_col, calc_rating );
        }
    }

    // Try stepping east
    if( start_col < topo_map[ 0 ].size() - 1 )
    {
        if( topo_map[ start_row ][ start_col + 1 ] ==
            ( topo_map[ start_row ][ start_col ] + 1 ) )
        {
            sum_scores += find_path_step( topo_map, start_row, start_col + 1, calc_rating );
        }
    }

    // Try stepping west
    if( 0 < start_col )
    {
        if( topo_map[ start_row ][ start_col - 1 ] ==
            ( topo_map[ start_row ][ start_col ] + 1 ) )
        {
            sum_scores += find_path_step( topo_map, start_row, start_col - 1, calc_rating );
        }
    }

    return sum_scores;
}

int main()
{
    std::vector<std::vector<int32_t>> topo_map;
    if( !read_input( topo_map ) )
    {
        return -1;
    }

    int32_t sum_scores = 0;
    int32_t sum_ratings = 0;
    for( size_t row = 0; row < topo_map.size(); ++row )
    {
        for( size_t col = 0; col < topo_map[ 0 ].size(); ++col )
        {
            if( 0 == topo_map[ row ][ col ] )
            {
                // Send a copy of the map - find path step will modify.
                std::vector<std::vector<int32_t>> temp_map = topo_map;
                sum_scores += find_path_step( temp_map, row, col, false );

                temp_map = topo_map;
                sum_ratings += find_path_step( temp_map, row, col, true );
            }
        }
    }

    std::cout << "The sum of the path scores is: " << sum_scores << "\n";
    std::cout << "The sum of the path ratings is: " << sum_ratings << "\n";
}
