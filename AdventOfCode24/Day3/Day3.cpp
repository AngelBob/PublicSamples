// Day3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

static bool read_input( std::vector<std::pair<int32_t, int32_t>>& mul_values )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is named "Reports.txt" and
    // is present in the same folder as the executable.
    std::ifstream file( ".\\CorruptInput.txt" );

    // Step 2: read the data.
    // Integers in the file represent levels.
    // Each line contains multiple levels separated by a space.
    static const std::regex mul_regex( "mul\\(([0-9]+),([0-9]+)\\)" );
    std::string line;
    while( std::getline( file, line ) )
    {
        std::string::const_iterator line_start( line.cbegin() );
        std::smatch mul_match;
        while( std::regex_search( line_start, line.cend(), mul_match, mul_regex ) )
        {
            mul_values.emplace_back(
                std::make_pair( std::atoi( mul_match[ 1 ].str().c_str() ), std::atoi( mul_match[ 2 ].str().c_str() ) )
            );
            line_start = mul_match.suffix().first;
        }
    }

    // Step 3: return success or failure.
    return ( 0 != mul_values.size() );
}

static int64_t do_mul( std::vector<std::pair<int32_t, int32_t>>& mul_values )
{
    int64_t result = 0;
    for( auto& pair : mul_values )
    {
        result += ( pair.first * pair.second );
    }

    return result;
}

int main()
{
    std::vector<std::pair<int32_t, int32_t>> mul_values;
    if( !read_input( mul_values) )
    {
        return -1;
    }

    int64_t result = do_mul( mul_values );
    std::cout << "Extracted result = " << result << "\n";
}
