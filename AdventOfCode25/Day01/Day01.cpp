// Day01.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <list>
#include <string>

static const size_t combo_numbers = 100;
static const int64_t start_pos = 50;

static bool read_input( std::list<std::pair<char, uint64_t>>& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file present in the same folder as the executable.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read each line and create the data pair.
    char dir;
    uint64_t number;
    size_t count = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        dir = line[ 0 ];
        number = std::atoi( line.c_str() + 1 );
        input.emplace_back( std::make_pair( dir, number ) );

        ++count;
    }

    // Step 3: return success or failure.
    return ( count == input.size() );
}

static size_t count_zeros( const std::list<std::pair<char, uint64_t>>& input )
{
    // Iterate through the lock operations updating the next position.
    int64_t position = start_pos;
    size_t zeros = 0;
    for( const auto& [ dir, count ] : input )
    {
        // If the number of clicks is more than the number of lock numbers
        // take the modulus and move that number to find the next position.
        int64_t clicks = count;
        if( std::abs( clicks ) > combo_numbers )
        {
            clicks %= combo_numbers;
        }

        // Move the dial the appropriate number of clicks in the correct
        // direction.
        switch( dir )
        {
        case 'R':
            position += clicks;
            break;
        case 'L':
            position -= clicks;
            break;
        }

        // Check if the dial has "wrapped" and update the next position.
        if( 0 > position )
        {
            position += combo_numbers;
        }
        else if( combo_numbers < position )
        {
            position -= combo_numbers;
        }

        // Count the number of times the dial lands on zero.
        if( 0 == position || combo_numbers == position )
        {
            ++zeros;
            position = 0;
        }
    }

    return zeros;
}

int main()
{
    std::list<std::pair<char, uint64_t>> input;

    if( !read_input( input ) )
    {
        return -1;
    }

    // Count the number of times the combination lands on 0
    size_t count = count_zeros( input );

    std::cout << "Password is: " << count << std::endl;
}
