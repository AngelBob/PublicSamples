// Day6.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
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
    std::vector<std::vector<std::pair<char, bool>>>& plot_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Plots.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    size_t height = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        std::vector<std::pair<char, bool>> row_data;
        for( char c : line )
        {
            row_data.emplace_back( std::make_pair( c, false ) );
        }
        plot_map.emplace_back( std::move( row_data ) );

        ++height;
    }

    // Step 3: return success or failure.
    return ( height == plot_map[ 0 ].size() ); // Expecting a square grid
}

static bool is_perimeter(
    const char plot_id,
    const std::vector<std::vector<std::pair<char, bool>>>& plot_map,
    const int32_t row,
    const int32_t col
)
{
    bool perimeter = ( row < 0 || plot_map.size() <= row );
    if( !perimeter )
    {
        perimeter = ( col < 0 || plot_map[ row ].size() <= col );
    }

    if( !perimeter )
    {
        perimeter = ( plot_map[ row ][ col ].first != plot_id );
    }

    return perimeter;
}

static std::pair<size_t, size_t> flood_plot(
    std::vector<std::vector<std::pair<char, bool>>>& plot_map,
    const char plot_id,
    const int32_t start_row,
    const int32_t start_col
)
{
    size_t perimeter = 0;
    size_t area = 0;

    std::list<std::pair<int32_t, int32_t>> flood_wave{{ start_row, start_col }};
    std::list<std::pair<int32_t, int32_t>>::iterator flood_front = flood_wave.begin();
    while( flood_wave.end() != flood_front )
    {
        // Add area and perimeter for this section of the plot.
        int32_t this_row = flood_front->first;
        int32_t this_col = flood_front->second;

        if( !plot_map[ this_row ][ this_col ].second )
        {
            const std::array<std::pair<int32_t, int32_t>, WALK_DIR::WALK_END> flood_paths{{
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_NORTH ].y_inc, this_col + walk_data[ WALK_DIR::WALK_NORTH ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_EAST  ].y_inc, this_col + walk_data[ WALK_DIR::WALK_EAST  ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_SOUTH ].y_inc, this_col + walk_data[ WALK_DIR::WALK_SOUTH ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_WEST  ].y_inc, this_col + walk_data[ WALK_DIR::WALK_WEST  ].x_inc ),
            }};

            ++area;
            for( const auto& flood_path : flood_paths )
            {
                int32_t check_row = flood_path.first;
                int32_t check_col = flood_path.second;

                if( is_perimeter( plot_id, plot_map, check_row, check_col ) )
                {
                    ++perimeter;
                }
                else if( plot_map[ check_row ][ check_col ].first == plot_id )
                {
                    flood_wave.insert( flood_wave.end(), std::make_pair( flood_path.first, flood_path.second ) );
                }
            }

            // Mark this section as counted.
            plot_map[ this_row ][ this_col ].second = true;
        }

        // Move to the next plot section
        flood_wave.pop_front();
        flood_front = flood_wave.begin();
    }

    return std::make_pair( perimeter, area );
}

int main()
{
    std::vector<std::vector<std::pair<char, bool>>> plot_map;
    if( !read_input( plot_map ) )
    {
        return -1;
    }

    size_t fence_cost = 0;
    int32_t row = 0, col = 0;

    while( row < plot_map.size() &&
           col < plot_map[ row ].size() )
    {
        char plot_id = plot_map[ row ][ col ].first;

        std::pair<size_t, size_t> plot_data = flood_plot( plot_map, plot_id, row, col );
        fence_cost += plot_data.first * plot_data.second;

        while( plot_map[ row ][ col ].second )
        {
            ++col;
            if( col == plot_map[ row ].size() )
            {
                col = 0;
                ++row;
                if( row == plot_map.size() )
                {
                    break;
                }
            }
        }
    }

    std::cout << "Total fence cost is " << fence_cost << "\n";
}
