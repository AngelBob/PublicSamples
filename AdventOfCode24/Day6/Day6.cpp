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

static std::pair<size_t, size_t> do_guard_walk(
    std::vector<std::string>& grid,
    std::tuple<size_t, size_t, enum WALK_DIR>& guard_status
)
{
    const size_t max_idx = grid.size();

    // Keep track of the directions in which the guard crosses each location.
    std::vector<std::vector<std::array<bool, WALK_DIR::WALK_END>>> visited_dir;
    for( size_t row = 0; row < grid.size(); ++row )
    {
        std::vector<std::array<bool, WALK_DIR::WALK_END>> col_dirs;
        for( size_t col = 0; col < grid.size(); ++col )
        {
            col_dirs.emplace_back( std::array<bool, WALK_DIR::WALK_END>{{ false, false, false, false }} );
        }

        visited_dir.emplace_back( col_dirs );
    }

    size_t location_count = 1; // Mark the starting location as visited.
    size_t route_loop_count = 0; // Count the number of route loop options.
    while( 1 )
    {
        const walk_data_t this_walk = walk_data[ std::get<2>( guard_status ) ];
        size_t col = std::get<0>( guard_status );
        size_t row = std::get<1>( guard_status );

        // Mark the guard's current location as visited.
        grid[ row ][ col ] = this_walk.dir_indicator;

        // Mark the direction in which the guard is traveling.
        visited_dir[ row ][ col ][ this_walk.dir ] = true;

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

        // Check if the guard can be placed in a route loop. Route loops occur
        // if the guard's direction can be changed and the new route includes
        // a location that the guard has already visited while traveling in the
        // same direction.
        std::vector<std::vector<std::array<bool, WALK_DIR::WALK_END>>> visited_loop_dir = visited_dir;

        size_t dir_t = ( static_cast<size_t>( std::get<2>( guard_status ) ) + 1 ) % WALK_DIR::WALK_END;
        size_t col_t = col - this_walk.x_inc;
        size_t row_t = row - this_walk.y_inc;
        while( 1 )
        {
            visited_loop_dir[ row_t ][ col_t ][ dir_t ] = true;

            col_t += walk_data[ dir_t ].x_inc;
            row_t += walk_data[ dir_t ].y_inc;

            if( col_t < 0 || max_idx <= col_t ||
                row_t < 0 || max_idx <= row_t )
            {
                // Guard is going to move off the map, no route loop possible.
                break;
            }
            else if( '#' == grid[ row_t ][ col_t ] )
            {
                // Guard is about to hit an obstacle, rotate path 90 degrees and
                // try stepping in the new direction.
                dir_t = ( dir_t + 1 ) % WALK_DIR::WALK_END;
                continue;
            }
            else if( visited_loop_dir[ row_t ][ col_t ][ dir_t ] )
            {
                // Route loop created!
#ifdef _DEBUG
                // Print the location
                std::cout << row << "," << col << std::endl;
#endif
                ++route_loop_count;
                break;
            }
        }
    }

    return std::make_pair( location_count, route_loop_count );
}

int main()
{
    std::vector<std::string> grid;
    std::tuple<size_t, size_t, enum WALK_DIR> guard_status = std::make_tuple( 0, 0, WALK_DIR::WALK_NORTH );
    if( !read_input( grid, guard_status ) )
    {
        return -1;
    }

    std::pair<size_t, size_t> counts = do_guard_walk( grid, guard_status );
    std::cout << "Guard touches " << counts.first << " distinct locations and\n";
    std::cout << "there are " << counts.second << " route loop options." << std::endl;
}
