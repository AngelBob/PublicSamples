// Day8.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static bool read_input(
    std::multimap<char, std::pair<int32_t, int32_t>>& antenna_locations,
    int32_t& extent // Makes a square grid.
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2: read each line and map the locations of antennae.
    int32_t y = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        int32_t x = 0;
        for( auto& letter : line )
        {
            if( '.' != letter )
            {
                antenna_locations.emplace( letter, std::make_pair( x, y ) );
            }
            ++x;
        }
        ++y;
    }
    extent = y;

    // Flip the y values, so that y=0 is at the "bottom" of the visual data.
    for( auto& item : antenna_locations )
    {
        item.second.second = extent - item.second.second - 1;
    }

    // Step 3: return success or failure.
    return true;
}

static void find_antinodes(
    const std::multimap<char, std::pair<int32_t, int32_t>>& antenna_locations,
    const int32_t max_idx,
    std::map<std::pair<int32_t, int32_t>, bool>& antinodes )
{
    // Walk the list of antenna locations and find the anti-node locations for
    // each pair of antennae.
    std::multimap<char, std::pair<int32_t, int32_t>>::const_iterator next = antenna_locations.cbegin();
    std::multimap<char, std::pair<int32_t, int32_t>>::const_iterator start = next++;
    while( antenna_locations.cend() != std::next( start, 2 ) )
    {
        // Find a pair of antennae
        while( antenna_locations.cend() == next ||
               next->first != start->first )
        {
            ++start;
            next = std::next( start );
        }

        // Calculate the slope between the two locations
        int32_t delta_y = next->second.second - start->second.second;
        int32_t delta_x = next->second.first - start->second.first;
        int32_t multiplier_x = start->second.first > next->second.first ? 1 : -1;
        int32_t multiplier_y = start->second.second > next->second.second ? 1 : -1;

        // Calculate the locations of the two antinodes
        int32_t node0_x, node0_y, node1_x, node1_y;

        node0_x = start->second.first + ( std::abs( delta_x ) * multiplier_x );
        node0_y = start->second.second + ( std::abs( delta_y ) * multiplier_y );

        node1_x = next->second.first - ( std::abs( delta_x ) * multiplier_x );
        node1_y = next->second.second - ( std::abs( delta_y ) * multiplier_y );

        if( 0 <= node0_x && node0_x < max_idx &&
            0 <= node0_y && node0_y < max_idx )
        {
            antinodes.try_emplace( std::make_pair( node0_x, node0_y ), true );
        }

        if( 0 <= node1_x && node1_x < max_idx &&
            0 <= node1_y && node1_y < max_idx )
        {
            antinodes.try_emplace( std::make_pair( node1_x, node1_y ), true );
        }

        ++next;
    }
}

int main()
{
    std::multimap<char, std::pair<int32_t, int32_t>> antenna_locations;
    int32_t max_idx;
    if( !read_input( antenna_locations, max_idx ) )
    {
        return -1;
    }

    std::map<std::pair<int32_t, int32_t>, bool> antinodes;
    find_antinodes( antenna_locations, max_idx, antinodes );
    std::cout << "There are " << antinodes.size() << " unique antinodes on the map.\n";
}
