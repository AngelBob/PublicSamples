// Day6.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

enum WALK_DIR
{
    WALK_START = 0,
    WALK_NORTH = 0,
    WALK_EAST,
    WALK_SOUTH,
    WALK_WEST,
    WALK_END
};

typedef struct walk_data
{
    enum WALK_DIR  dir;
    char           dir_indicator;
    int8_t         x_inc;
    int8_t         y_inc;
} walk_data_t;

static constexpr std::array<walk_data_t, WALK_END> walk_data{{
    { WALK_DIR::WALK_NORTH, '^',  0, -1},
    { WALK_DIR::WALK_EAST,  '>',  1,  0},
    { WALK_DIR::WALK_SOUTH, 'v',  0,  1},
    { WALK_DIR::WALK_WEST,  '<', -1,  0},
}};

static bool read_input(
    std::vector<std::string>& grid,
    std::tuple<size_t, size_t, enum WALK_DIR>& guard_status
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    size_t height = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        grid.emplace_back( line );

        // Assumes guard always starts by walking north.
        size_t guard_x = line.find( '^' );
        if( line.npos != guard_x )
        {
            std::get<0>( guard_status ) = guard_x;
            std::get<1>( guard_status ) = height;
            std::get<2>( guard_status ) = WALK_DIR::WALK_NORTH;
        }

        ++height;
    }

    // Step 3: return success or failure.
    return ( height == grid[ 0 ].size() ); // Expecting a square grid
}

static size_t do_guard_walk(
    std::vector<std::string>& grid,
    std::tuple<size_t, size_t, enum WALK_DIR>& guard_status
)
{
    const size_t max_idx = grid.size();

    size_t location_count = 1; // Mark the starting location as visited.
    while( 1 )
    {
        const walk_data_t this_walk = walk_data[ std::get<2>( guard_status ) ];
        size_t col = std::get<0>( guard_status );
        size_t row = std::get<1>( guard_status );

        // Mark the guard's current location as visited.
        grid[ row ][ col ] = this_walk.dir_indicator;

        // Try to move the guard
        col += this_walk.x_inc;
        row += this_walk.y_inc;

        if( col < 0 || max_idx <= col ||
            row < 0 || max_idx <= row )
        {
            // Guard is going to move off the map
            break;
        }
        else if( '#' == grid[ row ][ col ] )
        {
            // Guard is about to hit an obstacle, rotate path 90 degrees and
            // try stepping in the new direction.
            size_t dir = std::get<2>( guard_status );
            dir = ( dir + 1 ) % WALK_DIR::WALK_END;

            std::get<2>( guard_status ) = static_cast< WALK_DIR >( dir );

            continue;
        }
        else
        {
            // Count distinct locations
            location_count += ( '.' == grid[ row ][ col ] ) ? 1 : 0;

            // Move the guard
            std::get<0>( guard_status ) = col;
            std::get<1>( guard_status ) = row;
        }
    }

    return location_count;
}

int main()
{
    std::vector<std::string> grid;
    std::tuple<size_t, size_t, enum WALK_DIR> guard_status = std::make_tuple( 0, 0, WALK_DIR::WALK_NORTH );
    if( !read_input( grid, guard_status ) )
    {
        return -1;
    }

    size_t count = do_guard_walk( grid, guard_status );
    std::cout << "Guard touches " << count << " distinct locations\n";
}
