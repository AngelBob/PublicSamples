// Day4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

static bool read_input( std::vector<std::string>& grid )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\WordSearch_full.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    size_t width = 0, height = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        grid.emplace_back( line );
        ++height;
    }

    // Step 3: return success or failure.
    return ( height == grid[ 0 ].length() ); // Expecting a square grid
}

enum SCAN_DIR
{
    SCAN_START = 0,
    SCAN_NORTH = 0,
    SCAN_NORTHEAST,
    SCAN_EAST,
    SCAN_SOUTHEAST,
    SCAN_SOUTH,
    SCAN_SOUTHWEST,
    SCAN_WEST,
    SCAN_NORTHWEST,
    SCAN_END
};

typedef struct scan_data
{
    enum SCAN_DIR  dir;
    int8_t         x_inc;
    int8_t         y_inc;
} scan_data_t;

static const std::array<scan_data_t, SCAN_END> scan_data{{
    { SCAN_DIR::SCAN_NORTH,      0, -1 },
    { SCAN_DIR::SCAN_NORTHEAST,  1, -1 },
    { SCAN_DIR::SCAN_EAST,       1,  0 },
    { SCAN_DIR::SCAN_SOUTHEAST,  1,  1 },
    { SCAN_DIR::SCAN_SOUTH,      0,  1 },
    { SCAN_DIR::SCAN_SOUTHWEST, -1,  1 },
    { SCAN_DIR::SCAN_WEST,      -1,  0 },
    { SCAN_DIR::SCAN_NORTHWEST, -1, -1 },
}};

static size_t do_word_search(
    std::vector<std::string>& grid,
    const std::string word )
{
    size_t word_count = 0;
    int32_t row_base = 0;

    while( row_base < grid.size() )
    {
        int32_t col_base = 0;
        while( col_base < grid.size() )
        {
            for( const scan_data_t& scan : scan_data )
            {
                std::string::const_iterator w_it = word.cbegin();

                int32_t col = col_base;
                int32_t row = row_base;
                bool have_match = true;
                while( 1 )
                {
                    if( *w_it != grid[ row ][ col ] )
                    {
                        have_match = false;
                        break;
                    }

                    if( ++w_it == word.end() )
                    {
                        // Have a full match
                        break;
                    }

                    col += scan.x_inc;
                    row += scan.y_inc;
                    if( col < 0 || grid.size() <= col ||
                        row < 0 || grid.size() <= row )
                    {
                        // Walked off the edge of the grid, bail
                        have_match = false;
                        break;
                    }
                }

                if( have_match )
                {
                    // Found a match
                    ++word_count;
                }
            }
            ++col_base;
        }
        ++row_base;
    }

    return word_count;
}

int main()
{
    std::vector<std::string> grid;
    if( !read_input( grid ) )
    {
        return -1;
    }

    size_t count = do_word_search( grid, "XMAS" );
    std::cout << "Word appears " << count << " times.\n";
}
