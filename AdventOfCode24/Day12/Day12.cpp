// Day6.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <array>
#include <fstream>
#include <iostream>
#include <set>
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

static std::pair<size_t, size_t> count_perimeter_and_corners(
    const char plot_id,
    const std::vector<std::vector<std::pair<char, bool>>>& plot_map,
    const int32_t row,
    const int32_t col
)
{
    std::pair<size_t, size_t> perimeter_corners( 0, 0 );

    const std::array<std::pair<int32_t, int32_t>, WALK_DIR::WALK_END> neighbors{{
        std::make_pair( row + walk_data[ WALK_DIR::WALK_NORTH ].y_inc, col + walk_data[ WALK_DIR::WALK_NORTH ].x_inc ),
        std::make_pair( row + walk_data[ WALK_DIR::WALK_EAST  ].y_inc, col + walk_data[ WALK_DIR::WALK_EAST  ].x_inc ),
        std::make_pair( row + walk_data[ WALK_DIR::WALK_SOUTH ].y_inc, col + walk_data[ WALK_DIR::WALK_SOUTH ].x_inc ),
        std::make_pair( row + walk_data[ WALK_DIR::WALK_WEST  ].y_inc, col + walk_data[ WALK_DIR::WALK_WEST  ].x_inc ),
    }};

    // Check for perimeters
    bool n_perim = is_perimeter( plot_id, plot_map, neighbors[ WALK_DIR::WALK_NORTH ].first, neighbors[ WALK_DIR::WALK_NORTH ].second );
    bool e_perim = is_perimeter( plot_id, plot_map, neighbors[ WALK_DIR::WALK_EAST  ].first, neighbors[ WALK_DIR::WALK_EAST  ].second );
    bool s_perim = is_perimeter( plot_id, plot_map, neighbors[ WALK_DIR::WALK_SOUTH ].first, neighbors[ WALK_DIR::WALK_SOUTH ].second );
    bool w_perim = is_perimeter( plot_id, plot_map, neighbors[ WALK_DIR::WALK_WEST  ].first, neighbors[ WALK_DIR::WALK_WEST  ].second );

    perimeter_corners.first += n_perim ? 1 : 0;
    perimeter_corners.first += e_perim ? 1 : 0;
    perimeter_corners.first += s_perim ? 1 : 0;
    perimeter_corners.first += w_perim ? 1 : 0;

    // Check for corners
    // Concave
    perimeter_corners.second += ( n_perim && w_perim ) ? 1 : 0;
    perimeter_corners.second += ( n_perim && e_perim ) ? 1 : 0;
    perimeter_corners.second += ( s_perim && e_perim ) ? 1 : 0;
    perimeter_corners.second += ( s_perim && w_perim ) ? 1 : 0;

    // Convex
    perimeter_corners.second += ( !n_perim && !w_perim && ( 0 <= row - 1 ) && ( 0 <= col - 1 ) && ( plot_map[ row - 1 ][ col - 1 ].first != plot_id ) ) ? 1 : 0;
    perimeter_corners.second += ( !n_perim && !e_perim && ( 0 <= row - 1 ) && ( col + 1 < plot_map[ row - 1 ].size() ) && ( plot_map[ row - 1 ][ col + 1 ].first != plot_id ) ) ? 1 : 0;
    perimeter_corners.second += ( !s_perim && !e_perim && ( row + 1 < plot_map.size() ) && ( col + 1 < plot_map[ row + 1 ].size() ) && ( plot_map[ row + 1 ][ col + 1 ].first != plot_id ) ) ? 1 : 0;
    perimeter_corners.second += ( !s_perim && !w_perim && ( row + 1 < plot_map.size() ) && ( 0 <= col - 1 ) && ( plot_map[ row + 1 ][ col - 1 ].first != plot_id ) ) ? 1 : 0;

    return perimeter_corners;
}

static std::pair<std::pair<size_t, size_t>, size_t> flood_plot(
    std::vector<std::vector<std::pair<char, bool>>>& plot_map,
    const char plot_id,
    const int32_t start_row,
    const int32_t start_col
)
{
    std::pair<size_t, size_t> total_perimeter_and_corners( 0, 0 );

    std::set<std::pair<int32_t, int32_t>> flood_wave = {{ start_row, start_col }};
    std::set<std::pair<int32_t, int32_t>>::iterator flood_front = flood_wave.begin();
    while( flood_wave.end() != flood_front )
    {
        // Add area and perimeter for this section of the plot.
        int32_t this_row = flood_front->first;
        int32_t this_col = flood_front->second;

        if( !plot_map[ this_row ][ this_col ].second )
        {
            std::pair<size_t, size_t> perimeter_corners = count_perimeter_and_corners( plot_id, plot_map, this_row, this_col );
            total_perimeter_and_corners.first += perimeter_corners.first;
            total_perimeter_and_corners.second += perimeter_corners.second;

            const std::array<std::pair<int32_t, int32_t>, WALK_DIR::WALK_END> flood_paths{{
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_EAST  ].y_inc, this_col + walk_data[ WALK_DIR::WALK_EAST  ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_SOUTH ].y_inc, this_col + walk_data[ WALK_DIR::WALK_SOUTH ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_WEST  ].y_inc, this_col + walk_data[ WALK_DIR::WALK_WEST  ].x_inc ),
                std::make_pair( this_row + walk_data[ WALK_DIR::WALK_NORTH ].y_inc, this_col + walk_data[ WALK_DIR::WALK_NORTH ].x_inc ),
            }};

            for( const auto& flood_path : flood_paths )
            {
                int32_t check_row = flood_path.first;
                int32_t check_col = flood_path.second;

                if( !is_perimeter( plot_id, plot_map, check_row, check_col ) &&
                    plot_map[ check_row ][ check_col ].first == plot_id &&
                    !plot_map[ check_row ][ check_col ].second )
                {
                    flood_wave.insert( flood_wave.end(), std::make_pair( flood_path.first, flood_path.second ) );
                }
            }

            // Mark this section as counted.
            plot_map[ this_row ][ this_col ].second = true;

            // Reset, since insert may have inserted at the front
            flood_front = flood_wave.begin();
        }
        else
        {
            // Move to the next plot section
            ++flood_front;
        }
    }

    return std::make_pair( total_perimeter_and_corners, flood_wave.size() );
}

int main()
{
    std::vector<std::vector<std::pair<char, bool>>> plot_map;
    if( !read_input( plot_map ) )
    {
        return -1;
    }

    size_t fence_cost = 0;
    size_t discount_cost = 0;
    int32_t row = 0, col = 0;

    while( row < plot_map.size() &&
           col < plot_map[ row ].size() )
    {
        char plot_id = plot_map[ row ][ col ].first;

        std::pair<std::pair<size_t, size_t>, size_t> plot_data = flood_plot( plot_map, plot_id, row, col );
        fence_cost += plot_data.first.first * plot_data.second;
        discount_cost += plot_data.first.second * plot_data.second;

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

    std::cout << "Total base fence cost is " << fence_cost << "\n";
    std::cout << "Total discount cost is " << discount_cost << std::endl;
}
