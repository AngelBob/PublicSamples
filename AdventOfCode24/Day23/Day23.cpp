// Day23.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

static bool read_input(
    std::map<std::string, std::set<std::string>>& lan_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Lan.txt");

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    while( std::getline( file, line ) )
    {
        std::string first( { line[ 0 ], line[ 1 ] } );
        std::string second( { line[ 3 ], line[ 4 ] } );

        lan_map[ first ].insert( second );
        lan_map[ second ].insert( first );
    }

    // Step 3: return success or failure.
    return ( 0 != lan_map.size() );
}

static void map_t_lans(
    const std::map<std::string, std::set<std::string>>& lan_maps,
    std::set<std::vector<std::string>>& lans_with_t
)
{
    for( const std::pair<std::string, std::set<std::string>>& t_base : lan_maps )
    {
        // All computers are in the map, skip those that don't start with "t".
        if( 't' != t_base.first[ 0 ] )
        {
            continue;
        }

        // For all t? computers, search their links for a matching t?.
        for( const std::string& link : t_base.second )
        {
            for( const std::string& loop : lan_maps.at( link ) )
            {
                if( t_base.first != loop && lan_maps.at( loop ).contains( t_base.first ) )
                {
                    std::vector<std::string> temp{ t_base.first, link, loop };
                    std::ranges::sort( temp );
                    if( lans_with_t.contains( temp ) )
                    {
                        continue;
                    }
                    lans_with_t.insert( temp );
                }
            }
        }
    }
}

int main()
{
    std::map<std::string, std::set<std::string>> lan_maps;
    if( !read_input( lan_maps ) )
    {
        return -1;
    }

    std::set<std::vector<std::string>> lans_with_t;
    map_t_lans( lan_maps, lans_with_t );
    std::cout << "There are " << std::to_string( lans_with_t.size() ) << " LANs with a t? computer.\n";
}
