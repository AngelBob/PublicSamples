// Day07.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

static bool read_input(
    std::vector<std::string>& input1,
    int16_t& input2
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input.txt" );

    // Step 2: read the lines and create the input data.
    bool have_start = false;
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        if( !have_start )
        {
            size_t x = line.find( 'S' );
            if( x != std::string::npos )
            {
                have_start = true;
                input2 = static_cast<int16_t>( x );
            }
            line[ x ] = '.';
        }

        input1.emplace_back( line );
    }

    // Step 3: return success or failure.
    return true;
}

static uint64_t traverse_tachyon_manifold(
    std::vector<std::string>& map,
    std::pair<int16_t, int16_t> head_xy
)
{
    static std::map<std::pair<int16_t, int16_t>, uint64_t> cache;

    // Bounds check to see if this head has exited the manifold.
    if( head_xy.first < 0 ||
        map[ 0 ].size() <= head_xy.first ||
        map.size() <= head_xy.second )
    {
        return 0;
    }

    // Check if this beam position has already been checked.
    if( '|' == map[ head_xy.second ][ head_xy.first ] )
    {
        return 0;
    }

    std::pair<int16_t, int16_t> key = std::make_pair( head_xy.first, head_xy.second );
    if( 0 != cache[ key ] )
    {
        return cache[ key ];
    }

    uint64_t splits = 0;

    // Map layout is yx, so reverse the indices of the head pointer.
    if( '^' == map[head_xy.second][head_xy.first] )
    {
        // Beam has hit a splitter.
        splits  = 1;
        splits += traverse_tachyon_manifold( map, std::make_pair( head_xy.first - 1, head_xy.second ) );
        splits += traverse_tachyon_manifold( map, std::make_pair( head_xy.first + 1, head_xy.second ) );
    }
    else
    {
        // Mark this beam position as checked.
        map[ head_xy.second ][ head_xy.first ] = '|';

        splits += traverse_tachyon_manifold( map, std::make_pair( head_xy.first, head_xy.second + 1 ) );
    }

    cache[ key ] = splits;

    return cache[ key ];
}

int main()
{
    std::vector<std::string> input1;
    int16_t input2;
    if( !read_input( input1, input2 ) )
    {
        return -1;
    }

    uint64_t splits = traverse_tachyon_manifold( input1, std::make_pair( input2, 0 ) );
    std::cout << "The beam is split " << splits << " times." << std::endl;
}
