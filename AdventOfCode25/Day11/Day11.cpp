// Day10.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static bool read_input(
    const std::string& filename,
    std::map<uint32_t, std::vector<uint32_t>>& input
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( filename );

    // Step 2: read the lines and create the input data.
    std::string token;
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        // Directly convert the three letter string into an unsigned integer.
        // NOTE: the character storage is little-endian on x86, so will be
        // backwards after conversion to 32-bit integer:
        // "you" == "uoy" == 0x00756f79 after conversion.
        std::string base = line.substr( 0, 3 );
        uint32_t base_i = *( reinterpret_cast<uint32_t*>( base.data() ) );

        std::stringstream ss( line.substr( 5 ) );
        std::string target;
        std::vector<uint32_t> targets;
        while( ss >> target )
        {
            // See above note about endian-ness...
            targets.emplace_back(
                *( reinterpret_cast<uint32_t*>( target.data() ) ) );
        }

        input.emplace( std::make_pair( base_i, targets ) );
    }

    // Step 3: return success or failure.
    return true;
}

static std::map<uint32_t, size_t> cache;

static size_t build_paths(
    std::map<uint32_t, std::vector<uint32_t>>& input,
    const uint32_t start,
    const uint32_t end
)
{
    std::map<uint32_t, size_t>::iterator it;
    it = cache.find( start );
    if( cache.end() != it )
    {
        return cache[ start ];
    }

    size_t exit_count = 0;
    for( const uint32_t next : input[ start ] )
    {
        if( end != next )
        {
            exit_count += build_paths( input, next, end );
        }
        else
        {
            exit_count = 1;
        }
    }

    cache[ start ] = exit_count;
    return exit_count;
}

/*
#define TEST_DATA_PART_1
/*/
#define TEST_DATA_PART_2
//*/

int main()
{
#if defined TEST_DATA_PART_1
    static const std::string filename( ".\\Data\\Input_test.txt" );
#elif defined TEST_DATA_PART_2
    static const std::string filename( ".\\Data\\Input_test2.txt" );
#else /* Use real data */
    static const std::string filename( ".\\Data\\Input.txt" );
#endif

    std::map<uint32_t, std::vector<uint32_t>> input;
    if( !read_input( filename, input ) )
    {
        return -1;
    }

    // Little-endian representation of the strings          "u o y"     "t u o"
    static const std::array<uint32_t, 2> part1_path = { 0x00756f79, 0x0074756f };
    size_t count = build_paths( input, part1_path[ 0 ], part1_path[ 1 ] );
    std::cout << "There are " << count << " possible paths." << std::endl;

    static const std::array<uint32_t, 4> part2_path = {
        0x00727673, /* rvs */
        0x00746666, /* tff */
        0x00636164, /* cad */
        0x0074756f  /* tuo */
    };
    count = 1;
    for( size_t idx = 0; idx < part2_path.size() - 1; ++idx )
    {
        cache.clear();
        count *= build_paths( input, part2_path[ idx ], part2_path[ idx + 1 ] );
    }
    std::cout << "There are " << count << " paths through fft & dac." << std::endl;
}
