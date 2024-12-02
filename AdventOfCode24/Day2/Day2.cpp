// Day2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static bool read_input( std::vector<std::vector<int32_t>>& reports )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is named "Reports.txt" and
    // is present in the same folder as the executable.
    std::ifstream file( ".\\Reports.txt" );

    // Step 2: read the data.
    // Integers in the file represent levels.
    // Each line contains multiple levels separated by a space.
    std::string line;
    while( std::getline( file, line ) )
    {
        reports.emplace_back();
        std::istringstream ss( line );
        while( !ss.eof() )
        {
            int32_t val;
            ss >> val;
            reports.back().emplace_back( val );
        }
    }

    // Step 3: return success or failure.
    return ( 0 != reports.size() );
}

static size_t count_safe( std::vector<std::vector<int32_t>>& reports )
{
    // A report is valid if:
    // 1) the levels are all increasing or all decreasing, and
    // 2) adjacent levels differ by at least one and at most three.
    size_t valid_cnt = 0;

    for( std::vector<int32_t> levels : reports )
    {
        size_t level_cnt = levels.size();
        assert( level_cnt > 1 );

        bool start_direction = levels[ 0 ] < levels[ 1 ];
        bool is_valid = true;

        for( size_t index = 0; is_valid && ( index < level_cnt - 1 ); ++index )
        {
            is_valid = ( start_direction == levels[ index ] < levels[ index + 1 ] ) &&
                        ( levels[ index ] != levels[ index + 1 ] ) &&
                        ( 3 >= std::abs( levels[ index ] - levels[ index + 1 ] ) );
        }

        valid_cnt += is_valid ? 1 : 0;
    }

    return valid_cnt;
}

int main()
{
    // Step 1: Read the input CSV
    std::vector<std::vector<int32_t>> reports;

    if( !read_input( reports ) )
    {
        return -1;
    }

    // Step 2: Count valid reports
    size_t valid_cnt = count_safe( reports );
    std::cout << "Valid report count = " << valid_cnt << "\n";
}
