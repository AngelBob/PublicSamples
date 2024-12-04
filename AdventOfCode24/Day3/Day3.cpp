// Day3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

static std::string::const_iterator get_next_disable(
    const std::string::const_iterator& line_start,
    const std::string::const_iterator& line_end )
{
    static const std::regex off( "don't\\(\\)" );

    std::string::const_iterator disable_start( line_end );
    std::smatch reg_match;
    if( std::regex_search( line_start, line_end, reg_match, off ) )
    {
        disable_start = reg_match.suffix().first;
    }

    return disable_start;
}

static std::string::const_iterator get_next_enable(
    const std::string::const_iterator& line_start,
    const std::string::const_iterator& line_end )
{
    static const std::regex on( "do\\(\\)" );

    std::string::const_iterator enable_start( line_end );
    std::smatch reg_match;
    if( std::regex_search( line_start, line_end, reg_match, on ) )
    {
        enable_start = reg_match.suffix().first;
    }

    return enable_start;
}

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

    bool is_enabled = true;
    std::string line;
    while( std::getline( file, line ) )
    {
        std::string::const_iterator line_start( line.cbegin() );
        std::string::const_iterator disable_start( line.cend() );

        if( !is_enabled )
        {
            line_start = get_next_enable( line.cbegin(), line.cend() );
            is_enabled = true;
        }
        disable_start = get_next_disable( line_start, line.cend() );

        std::smatch reg_match;
        while( std::regex_search( line_start, line.cend(), reg_match, mul_regex ) )
        {
            if( reg_match.suffix().first >= disable_start &&
                line.cend() != disable_start )
            {
                // mul() operator found after a don't() operation.
                // Look for the next do() operation from which to restart the
                // mul() operator search.
                line_start = get_next_enable( reg_match.suffix().first, line.cend() );

                // If there is no next do() operator, then disable the mul() accumator
                // for the beginning of the next line.
                if( line.cend() == line_start )
                {
                    is_enabled = false;
                    break;
                }

                // Also find the next don't() operation.
                disable_start = get_next_disable( line_start, line.cend() );

                // Restart search for mul() operators at new line_start (after next do())
                continue;
            }

            mul_values.emplace_back(
                std::make_pair( std::atoi( reg_match[ 1 ].str().c_str() ), std::atoi( reg_match[ 2 ].str().c_str() ) )
            );

            line_start = reg_match.suffix().first;
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
