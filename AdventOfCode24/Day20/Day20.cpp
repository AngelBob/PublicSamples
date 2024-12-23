// Day20.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <regex>
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
    std::vector<std::vector<std::pair<char, uint32_t>>>& map,
    std::pair<uint32_t, uint32_t>& start,
    std::pair<uint32_t, uint32_t>& end
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
        std::vector<std::pair<char, uint32_t>> line_data;
        for( const char c : line )
        {
            line_data.emplace_back( std::make_pair( c, 0 ) );
        }
        map.emplace_back( line_data );

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

static std::array<uint32_t, 4> get_cheat_values(
    const std::vector<std::vector<std::pair<char, uint32_t>>>& map,
    const std::pair<uint32_t, uint32_t>& current_location
)
{
    std::array<uint32_t, 4> savings{{
        static_cast<uint32_t>( -1 ),
        static_cast<uint32_t>( -1 ),
        static_cast<uint32_t>( -1 ),
        static_cast<uint32_t>( -1 )
    }};
    uint32_t row = current_location.second;
    uint32_t col = current_location.first;

    uint32_t this_step = map[ row ][ col ].second;

    // Check for cheat possibilities in every direction.
    for( const auto& walk : walk_data )
    {
        if( 0 <= ( row + ( 2 * walk.y_inc ) ) &&
            ( row + ( 2 * walk.y_inc ) ) < map.size() &&
            0 <= ( col + ( 2 * walk.x_inc ) ) &&
            ( col + ( 2 * walk.x_inc ) ) < map[ row ].size()
          )
        {
            if( '#' == map[ row + walk.y_inc ][ col + walk.x_inc ].first &&
                ( '.' == map[ row + ( 2 * walk.y_inc ) ][ col + ( 2 * walk.x_inc ) ].first ||
                  'S' == map[ row + ( 2 * walk.y_inc ) ][ col + ( 2 * walk.x_inc ) ].first ||
                  'E' == map[ row + ( 2 * walk.y_inc ) ][ col + ( 2 * walk.x_inc ) ].first ) &&
                this_step > map[ row + ( 2 * walk.y_inc ) ][ col + ( 2 * walk.x_inc ) ].second
              )
            {
                savings[ walk.dir ] = this_step - map[ row + ( 2 * walk.y_inc ) ][ col + ( 2 * walk.x_inc ) ].second - 2;
            }
        }
    }

    return savings;
}

static uint32_t walk_path(
    std::vector<std::vector<std::pair<char, uint32_t>>>& map,
    const std::pair<uint32_t, uint32_t>& start,
    const WALK_DIR came_from,
    std::map<uint32_t, uint32_t>& cheat_savings,
    const bool do_cheat
)
{
    uint32_t steps = 1;

    uint32_t x = start.first;
    uint32_t y = start.second;
    for( size_t idx = 0; idx < WALK_DIR::WALK_END; ++idx )
    {
        if( came_from == idx )
        {
            continue;
        }

        const walk_data_t& walk = walk_data[ idx ];
        x += walk.x_inc;
        y += walk.y_inc;

        if( (  do_cheat && map[ y ][ x ].first == 'E' ) ||
            ( !do_cheat && map[ y ][ x ].first == 'S' )
          )
        {
            break;
        }
        else if( map[ y ][ x ].first != '#' )
        {
            WALK_DIR from = static_cast<WALK_DIR>( ( idx + ( WALK_DIR::WALK_END / 2 ) ) % WALK_DIR::WALK_END );
            steps += walk_path( map, { x, y }, from,
                                cheat_savings, do_cheat );
            break; // only one path through the maze
        }

        x -= walk.x_inc;
        y -= walk.y_inc;
    }

    if( do_cheat )
    {
        std::array<uint32_t, 4> savings = get_cheat_values( map, { x, y } );
        for( uint32_t saving : savings )
        {
            if( -1 != saving )
            {
                ++cheat_savings[ saving ];
            }
        }
    }

    if( 0 == map[ y ][ x ].second )
    {
        // Note the step number for this location.
        map[ y ][ x ].second = steps - 1;
    }

    return steps;
}

int32_t main()
{
    std::vector<std::vector<std::pair<char, uint32_t>>> map;
    std::pair<uint32_t, uint32_t> start;
    std::pair<uint32_t, uint32_t> end;
    if( !read_input( map, start, end ) )
    {
        return -1;
    }

    // Get the base line time through the maze.
    // Need to walk from end to start to properly number the steps.
    std::map<uint32_t, uint32_t> cheat_savings;
    uint32_t base_t = walk_path( map, end, WALK_DIR::WALK_END, cheat_savings, false );
    map[ end.second ][ end.first ].second = base_t;
    std::cout << "Baseline time = " << std::to_string( base_t ) << " picoseconds.\n";

    // Check the savings with the cheats.
    static const uint32_t min_savings = 40;
    uint32_t total_cheats = 0;
    walk_path( map, start, WALK_DIR::WALK_END, cheat_savings, true );
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