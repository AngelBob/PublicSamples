// Day19.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static bool read_input(
    std::map<char, std::vector<std::string>>& towel_patterns,
    std::vector<std::string>& designs
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Towels.txt" );

    // Step 2: read each line and insert individual characters into the map.
    std::string line;
    while( std::getline( file, line, ',' ) )
    {
        if( line.npos == line.find( '\n' ) )
        {
            towel_patterns[ line[ 0 ] ].emplace_back( line );
        }
        else
        {
            std::stringstream split( line );
            if( std::getline( split, line ) )
            {
                towel_patterns[ line[ 0 ] ].emplace_back( line );
            }

            std::getline( split, line );
            while( std::getline( split, line ) )
            {
                designs.emplace_back( line );
            }
        }
    }

    for( auto& pair : towel_patterns )
    {
        std::sort( pair.second.begin(), pair.second.end(), std::greater<std::string>() );
    }

    // Step 3: return success or failure.
    return true;
}

static std::map<std::string, uint64_t> cache;

static uint64_t count_possibles(
    const std::map<char, std::vector<std::string>>& patterns,
    const std::string design
)
{
    if( 0 == cache[ design ] )
    {
        if( 0 == patterns.count( design[ 0 ] ) )
        {
            // This design is not possible
            cache[ design ] = -1;
        }
        else
        {
            // Find a combination of patterns that creates this design.
            std::list<std::pair<size_t, std::vector<std::string>>> possibles;
            for( auto& pattern : patterns.at( design[ 0 ] ) )
            {
                if( 0 == design.find( pattern, 0 ) )
                {
                    if( pattern.length() == design.length() )
                    {
                        ++cache[ design ];
                        break;
                    }

                    uint64_t possibles = count_possibles(
                        patterns,
                        { std::next( design.begin(), pattern.length() ), design.end() }
                    );

                    if( -1 != possibles )
                    {
                        cache[ design ] += possibles;
                    }
                }
            }
        }
    }

    if( 0 == cache[ design ] )
    {
        // No match for this design, it's not possible.
        cache[ design ] = -1;
    }

    return cache[ design ];
}

static uint64_t count_possible(
    const std::map<char, std::vector<std::string>>& patterns,
    const std::vector<std::string> designs
)
{
    uint64_t possible_designs = 0;
    for( const auto& design : designs )
    {
        if( -1 != count_possibles( patterns, design ) )
        {
            ++possible_designs;
        }
    }

    return possible_designs;
}

int main()
{
    std::map<char, std::vector<std::string>> patterns;
    std::vector<std::string> designs;
    if( !read_input( patterns, designs ) )
    {
        return -1;
    }

    uint64_t possibles = count_possible( patterns, designs );

    std::cout << "Possible designs = " << std::to_string( possibles ) << std::endl;
}
