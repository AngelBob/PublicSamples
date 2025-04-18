// Day14.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <tuple>
#include <vector>

static bool read_input(
    std::vector<std::tuple<int64_t, int64_t, int64_t, int64_t>>& robots
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Vectors.txt" );

    // Step 2: read the single line of input.
    static const std::regex point( "p=+([0-9]+),([0-9]+)" );
    static const std::regex slope( "v=([-+0-9]+),([-+0-9]+)" );

    std::string line;
    while( std::getline( file, line ) )
    {
        // Do a regex match to extract the machine data from the line.
        int64_t x, y, dx, dy;
        std::smatch reg_match;
        if( std::regex_search( line.cbegin(), line.cend(), reg_match, point ) )
        {
            x = stoll( reg_match[ 1 ] );
            y = stoll( reg_match[ 2 ] );
        }

        if( std::regex_search( line.cbegin(), line.cend(), reg_match, slope ) )
        {
            dx = stoll( reg_match[ 1 ] );
            dy = stoll( reg_match[ 2 ] );
        }

        robots.emplace_back( std::make_tuple( x, y, dx, dy ) );
    }

    // Step 3: return success or failure.
    return ( 0 != robots.size() );
}

static std::map<std::pair<int64_t, int64_t>, int64_t> move_robots(
    const std::vector<std::tuple<int64_t, int64_t, int64_t, int64_t>>& robots,
    const std::pair<int64_t, int64_t>& grid,
    const int64_t elapsed_time
)
{
    std::map<std::pair<int64_t, int64_t>, int64_t> locations;

    for( auto& robot : robots )
    {
        int64_t  x = std::get<0>( robot );
        int64_t dx = std::get<2>( robot );
        int64_t move_x = ( dx * elapsed_time ) % grid.first;

        int64_t new_x = move_x >= 0 ? ( x + move_x ) % grid.first : ( grid.first + x + move_x ) % grid.first;

        int64_t  y = std::get<1>( robot );
        int64_t dy = std::get<3>( robot );
        int64_t move_y = ( dy * elapsed_time ) % grid.second;

        int64_t new_y = move_y >= 0 ? ( y + move_y ) % grid.second : ( grid.second + y + move_y ) % grid.second;

        locations[ std::make_pair( new_x, new_y ) ]++;
    }

    return locations;
}

static uint64_t calculate_safety_factor(
    const std::map<std::pair<int64_t, int64_t>, int64_t>& locations,
    const std::pair<int64_t, int64_t>& grid
)
{
    std::pair<int64_t, int64_t> mids( grid.first / 2, grid.second / 2);

    std::tuple<int64_t, int64_t, int64_t, int64_t> quadrant_counts( 0, 0, 0, 0 );
    for( auto& location : locations )
    {
        if( location.first.first < mids.first )
        {
            // Left hand quadrants
            if( location.first.second < mids.second )
            {
                // Upper left
                std::get<0>( quadrant_counts ) += location.second;
            }
            else if( location.first.second > mids.second )
            {
                // Lower left
                std::get<2>( quadrant_counts ) += location.second;
            }
        }
        else if( location.first.first > mids.first )
        {
            // Right hand quadrants
            if( location.first.second < mids.second )
            {
                // Upper right
                std::get<1>( quadrant_counts ) += location.second;
            }
            else if( location.first.second > mids.second )
            {
                // Lower right
                std::get<3>( quadrant_counts ) += location.second;
            }
        }
    }

    int64_t safety_factor = std::get<0>( quadrant_counts );
    safety_factor *= std::get<1>( quadrant_counts );
    safety_factor *= std::get<2>( quadrant_counts );
    safety_factor *= std::get<3>( quadrant_counts );

    return safety_factor;
}

static bool find_christmas_tree(
    const std::map<std::pair<int64_t, int64_t>, int64_t>& locations
)
{
    // Look for a Christmas tree pattern inside of a square.
    // Basically, just find a big square, that's probably close enough.
    // The map key is pairs of x,y values, so there should be a pattern of
    // several keys with the same x values and y values increasing by one.
    size_t consecutive_x_count = 0;
    std::map<std::pair<int64_t, int64_t>, int64_t>::const_iterator iter = locations.begin();
    ++iter;
    while( locations.end() != iter )
    {
        if( iter->first.first == std::prev( iter )->first.first &&
            iter->first.second == std::prev( iter )->first.second + 1 )
        {
            ++consecutive_x_count;
        }
        else
        {
            consecutive_x_count = 0;
        }

        if( 25 == consecutive_x_count )
        {
            return true;
        }

        ++iter;
    }

    return false;
}

int main()
{
    std::vector<std::tuple<int64_t, int64_t, int64_t, int64_t>> robots;
    if( !read_input( robots ) )
    {
        return -1;
    }

    static const std::pair<int64_t, int64_t> grid( 11, 7 );

    std::map<std::pair<int64_t, int64_t>, int64_t> locations =
        move_robots( robots, grid, 100 );

    int64_t safety = calculate_safety_factor( locations, grid );
    std::cout << "The safety factor is " << safety << "\n";

    std::array<std::array<char, 101>, 103> picture;
    int64_t elapsed = 1;
    while( 1 )
    {
        locations = move_robots( robots, grid, elapsed++ );

        if( find_christmas_tree( locations ) )
        {
            for( auto& location : locations )
            {
                picture[ location.first.second ][ location.first.first ] = '*';
            }
            break;
        }
    }

    std::cout << "The Christmas Tree appears at time " << elapsed - 1 << " seconds" << std::endl;
    for( size_t y = 0; y < 103; ++y )
    {
        for( size_t x = 0; x < 101; ++x )
        {
            if( picture[ y ][ x ] == '*' )
            {
                std::cout << "*";
            }
            else
            {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
