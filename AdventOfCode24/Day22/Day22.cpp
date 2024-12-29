// Day22.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_input(
    std::vector<uint32_t>& secrets
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Secrets.txt");

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    while (std::getline(file, line))
    {
        secrets.emplace_back( std::stoul( line ) );;
    }

    // Step 3: return success or failure.
    return true;
}

static uint32_t calculate_secret(
    const uint32_t val
)
{
    uint32_t next;

    // Multiply by 64, mix, and prune.
    next = ( ( val << 6 ) ^ val ) & 0xFFFFFF;

    // Divide by 32 and mix
    next ^= ( next >> 5 );

    // Prune
    next &= 0xFFFFFF;

    // Multiply by 2048 and mix
    next ^= ( next << 11 );

    // Final prune
    next &= 0xFFFFFF;
    
    return next;
}

int main()
{
    std::vector<uint32_t> secrets;
    if( !read_input( secrets ) )
    {
        return -1;
    }

    uint64_t secret_sum = 0;
    for( const auto val : secrets )
    {
        uint32_t next = val;
        for (size_t round = 0; round < 2000; ++round)
        {
            next = calculate_secret( next );
        }
        secret_sum += next;
        std::cout << std::to_string( val ) << ": " << std::to_string( next ) << "\n";
    }
    std::cout << "Total of all secrets: " << std::to_string( secret_sum ) << std::endl;
}
