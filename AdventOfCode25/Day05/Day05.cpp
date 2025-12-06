// Day05.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_input(
    std::vector<std::pair<uint64_t, uint64_t>>& input1,
    std::vector<uint64_t>& input2
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the lines and create the input data.
    bool is_range = true;
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        if( line.empty() )
        {
            // Empty line is the delimiter between ranges and values.
            is_range = false;
            continue;
        }

        if( is_range )
        {
            // Read range data.
            size_t delimiter_pos = line.find( '-' );
            if( delimiter_pos == std::string::npos )
            {
                // Invalid range format.
                return false;
            }
            uint64_t start = std::stoull( line.substr( 0, delimiter_pos ) );
            uint64_t end = std::stoull( line.substr( delimiter_pos + 1 ) );
            input1.emplace_back( start, end );
        }
        else
        {
            // Read value data.
            uint64_t value = std::stoull( line );
            input2.emplace_back( value );
        }
    }

    // Step 3: return success or failure.
    return true;
}

static void combine_ranges(
    std::vector<std::pair<uint64_t, uint64_t>>& ranges
)
{
    // Sort by start value.
    std::sort( ranges.begin(), ranges.end() );

    // For each range, check if it overlaps with the next range and
    // combine them if so.
    std::vector<std::pair<uint64_t, uint64_t>>::iterator next = ranges.begin();
    std::vector<std::pair<uint64_t, uint64_t>>::iterator cur = next++;
    while( next != ranges.end() )
    {
        if( cur->second >= next->first )
        {
            // Ranges overlap, combine them.
            cur->second = std::max( cur->second, next->second );
            next = ranges.erase( next );
        }
        else
        {
            // Move to the next range.
            ++cur;
            ++next;
        }
    }
}

static bool is_value_in_range(
    const std::pair<uint64_t, uint64_t>& range,
    const uint64_t val
)
{
    return ( ( range.first <= val ) && ( val <= range.second ) );
}

int main()
{
    std::vector<std::pair<uint64_t, uint64_t>> input1;
    std::vector<uint64_t> input2;
    if( !read_input( input1, input2 ) )
    {
        return -1;
    }

    // Combine overlapping ranges.
    combine_ranges( input1 );

    // Count the number of fresh ingredients available.
    size_t count = 0;
    for( const auto& value : input2 )
    {
        for( const auto& range : input1 )
        {
            if( is_value_in_range( range, value ) )
            {
                ++count;
                break;
            }
        }
    }
    std::cout << "There are " << count;
    std::cout << " fresh ingredients available." << std::endl;

    return 0;
}
