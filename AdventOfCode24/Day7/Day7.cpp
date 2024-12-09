// Day7.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

static bool read_input(
    std::map<int32_t, std::vector<int32_t>>& calibration_data
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Calibrations.txt" );

    // Step 2: read each line to get the calibration result and input values.
    static const std::regex numbers( "([0-9]+)" );
    std::string line;
    while( std::getline( file, line ) )
    {

        // Do a regex match to extract the calibration data from the line.
        std::string::const_iterator line_start = line.cbegin();
        std::smatch reg_match;

        int32_t result;
        if( std::regex_search( line_start, line.cend(), reg_match, numbers ) )
        {
            result = stoi( reg_match[ 1 ] );
            line_start = reg_match.suffix().first;
        }

        std::vector<int32_t> inputs;
        while( line.cend() != line_start )
        {
            if( std::regex_search( line_start, line.cend(), reg_match, numbers ) )
            {
                int32_t input = stoi( reg_match[ 1 ] );
                inputs.emplace_back( input );

                line_start = reg_match.suffix().first;
            }
        }

        calibration_data.emplace( std::make_pair( result, inputs ) );
    }

    // Step 3: return success or failure.
    return ( 0 != calibration_data.size() );
}

int main()
{
    std::map<int32_t, std::vector<int32_t>> calibration_data;
    if( !read_input( calibration_data ) )
    {
        return -1;
    }
}
