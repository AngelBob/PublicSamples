// Day16.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <list>
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
    int8_t         x_inc;
    int8_t         y_inc;
} walk_data_t;

static constexpr std::array<walk_data_t, WALK_END> walk_data{{
    { WALK_DIR::WALK_NORTH,  0, -1},
    { WALK_DIR::WALK_EAST,   1,  0},
    { WALK_DIR::WALK_SOUTH,  0,  1},
    { WALK_DIR::WALK_WEST,  -1,  0},
}};

static bool read_input(
    std::vector<std::vector<std::pair<char, uint32_t>>>& map,
    std::tuple<size_t, size_t, enum WALK_DIR>& deer_status
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
        std::vector<std::pair<char, uint32_t>> line_data;
        for( const char c : line )
        {
            line_data.emplace_back( std::make_pair( c, 0 ) );
        }
        map.emplace_back( line_data );

        // Deer always starts by walking east.
        size_t deer_x = line.find( 'S' );
        if( line.npos != deer_x )
        {
            std::get<0>( deer_status ) = height;
            std::get<1>( deer_status ) = deer_x;
            std::get<2>( deer_status ) = WALK_DIR::WALK_EAST;
        }

        ++height;
    }

    // Step 3: return success or failure.
    return true;
}

static uint32_t solve_maze(
    std::vector<std::vector<std::pair<char, uint32_t>>>& map,
    const size_t start_row,
    const size_t start_col,
    const enum WALK_DIR start_dir,
    uint32_t path_score
)
{
    // Is the deer at the end?
    if( 'E' == map[ start_row ][ start_col ].first )
    {
        // Return the smaller path score.
        uint32_t current_end_score = map[ start_row ][ start_col ].second;
        if( !current_end_score ||
            ( path_score < current_end_score )
          )
        {
            map[ start_row ][ start_col ].second = path_score;
        }

        return map[ start_row ][ start_col ].second;
    }
    else if( 0 != map[ start_row ][ start_col ].second &&
                  map[ start_row ][ start_col ].second < path_score )
    {
        // It's a loop
        return static_cast< uint32_t >( -1 );
    }

    // Find all possible steps from here. Only possible to go straight or turn
    // 90 degrees to the left or right.
    std::list<std::tuple<size_t, size_t, enum WALK_DIR, uint32_t>> reset_points;

    size_t target_row_f = start_row + walk_data[ start_dir ].y_inc;
    size_t target_col_f = start_col + walk_data[ start_dir ].x_inc;
    bool forward = ( '#' != map[ target_row_f ][ target_col_f ].first );

    uint8_t turn_dir_r = ( start_dir + 1 ) % WALK_END;
    size_t target_row_r = start_row + walk_data[ turn_dir_r ].y_inc;
    size_t target_col_r = start_col + walk_data[ turn_dir_r ].x_inc;
    bool right_turn = ( '#' != map[ target_row_r ][ target_col_r ].first );

    uint8_t turn_dir_l = ( start_dir + WALK_END - 1 ) % WALK_END;
    size_t target_row_l = start_row + walk_data[ turn_dir_l ].y_inc;
    size_t target_col_l = start_col + walk_data[ turn_dir_l ].x_inc;
    bool left_turn = ( '#' != map[ target_row_l ][ target_col_l ].first );

    // Mark this space visited
    map[ start_row ][ start_col ].second = path_score;

    // Add one for the step and turn (if any)
    ++path_score;
    const uint32_t turn_score = path_score + 1000;

    // Make a move, prefer: forward, right, left.
    // If there is more than one option, cache the others.
    if( forward )
    {
        if( right_turn )
        {
            reset_points.emplace_front( std::make_tuple( target_row_r, target_col_r, static_cast<WALK_DIR>( turn_dir_r ), turn_score ) );
        }

        if( left_turn )
        {
            reset_points.emplace_front( std::make_tuple( target_row_l, target_col_l, static_cast<WALK_DIR>( turn_dir_l ), turn_score ) );
        }

        path_score = solve_maze( map, target_row_f, target_col_f, start_dir, path_score );
    }
    else if( right_turn )
    {
        if( left_turn )
        {
            reset_points.emplace_front( std::make_tuple( target_row_l, target_col_l, static_cast<WALK_DIR>( turn_dir_l ), turn_score ) );
        }

        path_score = solve_maze( map, target_row_r, target_col_r, static_cast<WALK_DIR>( turn_dir_r ), turn_score );
    }
    else if( left_turn )
    {
        path_score = solve_maze( map, target_row_l, target_col_l, static_cast<WALK_DIR>( turn_dir_l ), turn_score );
    }
    else
    {
        // Dead end.
        path_score = static_cast<uint32_t>( -1 );
    }

    // Process the walk starting at each reset point
    for( const auto& status : reset_points )
    {
        uint32_t reset_score = solve_maze( map, std::get<0>( status ), std::get<1>( status ), std::get<2>( status ), std::get<3>( status ) );
        if( reset_score < path_score )
        {
            path_score = reset_score;
        }
    }

    return path_score;
}

int main()
{
    std::vector<std::vector<std::pair<char, uint32_t>>> map;
    std::tuple<size_t, size_t, enum WALK_DIR> deer_status;
    if( !read_input( map, deer_status ) )
    {
        return -1;
    }

    uint32_t path_score = solve_maze(
        map,
        std::get<0>( deer_status ),
        std::get<1>( deer_status ),
        std::get<2>( deer_status ),
        0
    );

    std::cout << "The sum of the path scores is: " << path_score << "\n";
}