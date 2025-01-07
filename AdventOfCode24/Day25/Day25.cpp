// Day25.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_input(
    std::vector<std::array<uint8_t, 5>>& locks,
    std::vector<std::array<uint8_t, 5>>& keys
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\LockSets_full.txt");

    // Step 2: read each line and insert operations into the map.
    int8_t depth = 0;
    bool is_lock = false;
    std::string line;
    std::array<uint8_t, 5> depths{ 0 };

    while( std::getline( file, line ) )
    {
        if( line.empty() )
        {
            if( is_lock )
            {
                locks.emplace_back( std::move( depths ) );
            }
            else
            {
                keys.emplace_back( std::move( depths ) );
            }

            depths.fill( 0 );
            depth = 0;

            continue;
        }
        else if( 0 == depth )
        {
            if( "#####" == line )
            {
                is_lock = true;
            }
            else
            {
                is_lock = false;
            }
        }

        for( size_t idx = 0; idx < 5; ++idx )
        {
            if(  is_lock && '#' == line[ idx ] ||
                !is_lock && '.' == line[ idx ] )
            {
                ++depths[ idx ];
            }
        }

        ++depth;
    }

    // Step 3: return success or failure.
    return( ( 0 != locks.size() ) && ( 0 != keys.size() ) );
}

static size_t analyze_locks(
    const std::vector<std::array<uint8_t, 5>>& locks,
    const std::vector<std::array<uint8_t, 5>>& keys
)
{
    size_t count = 0;
    for( const auto& lock : locks )
    {
        for( const auto& key : keys )
        {
            if( key[ 0 ] >= lock[ 0 ] &&
                key[ 1 ] >= lock[ 1 ] &&
                key[ 2 ] >= lock[ 2 ] &&
                key[ 3 ] >= lock[ 3 ] &&
                key[ 4 ] >= lock[ 4 ] )
            {
                ++count;
            }
        }
    }

    return count;
}

int main()
{
    std::vector<std::array<uint8_t, 5>> locks;
    std::vector<std::array<uint8_t, 5>> keys;
    if( !read_input( locks, keys ) )
    {
        return -1;
    }

    size_t unique_pair_count = analyze_locks( locks, keys );
    std::cout << "The number of unique pairs is " << std::to_string( unique_pair_count ) << "\n";
}
