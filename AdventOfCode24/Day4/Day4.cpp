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

static bool have_match(
    std::vector<std::string>& grid,
    const std::string word,
    const int32_t row_base,
    const int32_t col_base,
    const enum SCAN_DIR dir,
    const bool forward_only = true
    )
{
    int32_t col = col_base;
    int32_t row = row_base;

    std::string diag_f;
    for( size_t letter = 0; letter < word.length(); ++letter )
    {
        if( col < 0 || grid.size() <= col ||
            row < 0 || grid.size() <= row )
        {
            // Walked off the edge of the grid, unable to match
            return false;
        }

        diag_f += grid[ row ][ col ];
        row += scan_data[ dir ].y_inc;
        col += scan_data[ dir ].x_inc;
    }
    std::string diag_r( diag_f.rbegin(), diag_f.rend() );

    return ( ( word == diag_f ) || ( !forward_only && ( word == diag_r ) ) );
}

static size_t do_word_search(
    std::vector<std::string>& grid,
    const std::string word )
{
    size_t word_count = 0;

    int32_t row = 0;
    while( row < grid.size() )
    {
        int32_t col = 0;
        while( col < grid.size() )
        {
            for( const scan_data_t& scan : scan_data )
            {
                if( have_match( grid, word, row, col, scan.dir ) )
                {
                    ++word_count;
                }
            }
            ++col;
        }
        ++row;
    }

    return word_count;
}

static size_t do_cross_search(
    std::vector<std::string>& grid,
    const std::string word )
{
    // Only works with odd length words...
    if( 0 == ( word.length() % 2 ) )
    {
        return 0;
    }

    size_t cross_count = 0;

    const int32_t half_size = static_cast<int32_t>( word.length() ) / 2;
    int32_t row_base = half_size;
    while( row_base < grid.size() - half_size )
    {
        int32_t col_base = 1;
        while( col_base < grid.size() - half_size )
        {
            std::string::const_iterator w_it = word.cbegin();
            std::advance( w_it, half_size );

            if( *w_it == grid[ row_base ][ col_base ] )
            {
                // Middle letter found, look for the word on the diagonal.
                if( have_match( grid, word, row_base + half_size, col_base - half_size, SCAN_NORTHEAST, false ) &&
                    have_match( grid, word, row_base - half_size, col_base - half_size, SCAN_SOUTHEAST, false ) )
                {
                    // Have the full cross
                    ++cross_count;
                }
            }
            ++col_base;
        }
        ++row_base;
    }

    return cross_count;
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

    count = do_cross_search( grid, "MAS" );
    std::cout << "Cross word appears " << count << " times." << std::endl;
}
