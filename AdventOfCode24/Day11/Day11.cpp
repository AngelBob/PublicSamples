// Day11.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

static bool read_input(
    std::vector<uint64_t>& starting_stones
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Stones.txt" );

    // Step 2: read the single line of input.
    static const std::regex numbers( "([0-9]+)" );
    std::string line;
    if( std::getline( file, line ) )
    {
        // Do a regex match to extract the stone values from the line.
        std::string::const_iterator line_start = line.cbegin();
        std::smatch reg_match;
        while( line.cend() != line_start )
        {
            if( std::regex_search( line_start, line.cend(), reg_match, numbers ) )
            {
                uint64_t stone = stoll( reg_match[ 1 ] );
                starting_stones.emplace_back( stone );

                line_start = reg_match.suffix().first;
            }
        }
    }

    // Step 3: return success or failure.
    return ( 0 != starting_stones.size() );
}

static bool has_even_length( const uint64_t value, uint64_t& length )
{
    // Count the number of digits in value.
    length = static_cast<uint64_t>( std::log10( value ) ) + 1;
    return ( 0 == ( length & 1 ) );
}

static uint64_t do_blink(
    const uint64_t value,
    const int32_t blink )
{
    // Return the number stones that this stone splits into.
    uint64_t stone_count = 0;
    if( 0 == blink )
    {
        // Stone is only itself
        return 1;
    }

    // Calculate the number of stones this value will spawn.
    uint64_t length;
    if( 0 == value )
    {
        stone_count = do_blink( 1, blink - 1 );
    }
    else if( has_even_length( value, length ) )
    {
        // Split the stone in two, left stone gets high digits right
        // stone gets low digits.
        uint64_t divisor = static_cast<uint64_t>( std::pow( 10, ( length / 2 ) ) );
        uint64_t left_digits = value / divisor;
        uint64_t right_digits = value % divisor;

        stone_count = ( do_blink( left_digits, blink - 1 ) +
                       do_blink( right_digits, blink - 1 ) );
    }
    else
    {
        stone_count = do_blink( value * 2024, blink - 1 );
    }

    return stone_count;
}

int main()
{
    std::vector<uint64_t> stones;
    if( !read_input( stones ) )
    {
        return -1;
    }

    uint64_t count = 0;
    for( uint64_t& value : stones )
    {
        count += do_blink( value, 25 );
    }
    std::cout << "After 25 blinks there are " << count << " stones.\n";
}
