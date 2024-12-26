// Day20.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    int8_t         x_inc;
    int8_t         y_inc;
} walk_data_t;

static constexpr std::array<walk_data_t, WALK_END> walk_data{ {
    { WALK_DIR::WALK_NORTH,  0, -1},
    { WALK_DIR::WALK_EAST,   1,  0},
    { WALK_DIR::WALK_SOUTH,  0,  1},
    { WALK_DIR::WALK_WEST,  -1,  0},
} };

static bool read_input(
    std::vector<std::string>& map,
    std::pair<int32_t, int32_t>& start,
    std::pair<int32_t, int32_t>& end
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    uint32_t height = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        map.emplace_back( line );

        // See if this line contains the runner start location.
        size_t start_x = line.find( 'S' );
        if( line.npos != start_x )
        {
            start = std::make_pair( static_cast<uint32_t>( start_x ), height );
        }

        // See if this line contains the runner end location.
        size_t end_x = line.find( 'E' );
        if( line.npos != end_x )
        {
            end = std::make_pair( static_cast<uint32_t>( end_x ), height );
        }

        ++height;
    }

    // Step 3: return success or failure.
    return true;
}

static std::map<uint32_t, uint32_t> check_cheat_mask(
    std::map<std::pair<int32_t, int32_t>, uint32_t>& path,
    const std::pair<int32_t, int32_t>& start,
    const std::pair<std::pair<int32_t, int32_t>, uint32_t>& current_location,
    const std::list<std::pair<int32_t, int32_t>>& cheat_mask

)
{
    std::map<uint32_t, uint32_t> savings;

    uint32_t row = current_location.first.second;
    uint32_t col = current_location.first.first;

    // Check for cheat possibilities based on the cheat mask.
    for( const auto& cheat_loc : cheat_mask )
    {
        int32_t new_row = row + cheat_loc.second;
        int32_t new_col = col + cheat_loc.first;

        uint32_t target_step = path[ { new_col, new_row } ];
        if( target_step )
        {
            int32_t cheat_distance = std::abs( cheat_loc.first ) + std::abs( cheat_loc.second );
            if (current_location.second + cheat_distance < target_step)
            {
                uint32_t delta = target_step - (current_location.second + cheat_distance);
                savings[delta]++;
            }
        }
        else if( std::make_pair( new_col, new_row ) != start )
        {
            path.erase( { new_col, new_row } );
        }

    }

    return savings;
}

static uint32_t build_path(
    std::vector<std::string>& map,
    const std::pair<int32_t, int32_t>& start,
    const WALK_DIR came_from,
    std::map<std::pair<int32_t, int32_t>, uint32_t>& path
)
{
    uint32_t steps = 1;

    int32_t x, y;
    for( const auto& walk : walk_data )
    {
        if( came_from == walk.dir )
        {
            continue;
        }

        x = start.first + walk.x_inc;
        y = start.second + walk.y_inc;
        if( map[ y ][ x ] == 'S' )
        {
            break;
        }
        else if( map[ y ][ x ] != '#' )
        {
            WALK_DIR from = static_cast<WALK_DIR>( ( walk.dir + ( WALK_DIR::WALK_END / 2 ) ) % WALK_DIR::WALK_END );
            steps += build_path( map, { x, y }, from, path );

            break; // only one path through the maze
        }
    }

    path.insert( std::make_pair(std::make_pair( x, y ), steps - 1 ) );

    return steps;
}

static void build_cheat_mask(
    std::list<std::pair<int32_t, int32_t>>& mask,
    const int32_t mask_size,
    const bool straight_only
)
{
    for( int32_t y = mask_size; y >= -mask_size; --y )
    {
        if (y >= 0)
        {
            for (int32_t x = -(mask_size - y); x <= mask_size - y; ++x)
            {
                // Skip the origin and the spaces directly around it.
                if( ( 0 == x && 1 == y ) ||
                    ( 0 == y && ( -1 == x || 0 == x || 1 == x ) )
                  )
                {
                    continue;
                }

                if( straight_only && ( x != 0 && y != 0 ) )
                {
                    continue;
                }

                mask.emplace_back(std::make_pair(x, y));
            }
        }
        else
        {
            for (int32_t x = -( mask_size + y ); x <= mask_size + y; ++x)
            {
                // Skip space below the origin.
                if( 0 == x && -1 == y )
                {
                    continue;
                }

                if( straight_only && ( x != 0 ) )
                {
                    continue;
                }

                mask.emplace_back(std::make_pair(x, y));
            }
        }
    }
}

static void get_cheat_values(
    std::map<std::pair<int32_t, int32_t>, uint32_t>& path,
    const std::pair<int32_t, int32_t> start,
    const std::list<std::pair<int32_t, int32_t>>& cheat_mask,
    std::map<uint32_t, uint32_t>& cheat_savings
)
{
    for( const auto& location : path )
    {
        std::map<uint32_t, uint32_t> cheat_values = check_cheat_mask( path, start, location, cheat_mask );
        for( auto cheat_it = cheat_values.begin(); cheat_values.end() != cheat_it; ++cheat_it )
        {
            cheat_savings[ cheat_it->first ] += cheat_it->second;
        }
    }
}

int32_t main()
{
    std::vector<std::string> map;
    std::pair<int32_t, int32_t> start;
    std::pair<int32_t, int32_t> end;
    if( !read_input( map, start, end ) )
    {
        return -1;
    }

    // Get the base line time and a map through the maze.
    std::map<std::pair<int32_t, int32_t>, uint32_t> path;
    uint32_t base_t = build_path( map, end, WALK_DIR::WALK_END, path );
    path[ end ] = base_t;
    std::cout << "Baseline time = " << std::to_string( base_t ) << " picoseconds.\n";

    // Build the cheat mask
    std::list<std::pair<int32_t, int32_t>> cheat_mask;
    build_cheat_mask( cheat_mask, 20, false );

    // Check the savings with the cheats.
    std::map<uint32_t, uint32_t> cheat_savings;
    get_cheat_values( path, start, cheat_mask, cheat_savings );

    static const uint32_t min_savings = 100;
    uint32_t total_cheats = 0;
    for( const auto& saving : cheat_savings )
    {
        if( saving.first >= min_savings )
        {
            total_cheats += saving.second;
        }
    }
    std::cout << "There are " << std::to_string( total_cheats ) <<
        " that save at least " << std::to_string( min_savings ) << " picoseconds." << std::endl;
}