// Day7.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
#include <regex>
#include <string>
#include <vector>

static bool read_input(
    std::map<size_t, std::vector<size_t>>& calibration_data
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Calibrations_full.txt" );

    // Step 2: read each line to get the calibration result and input values.
    static const std::regex numbers( "([0-9]+)" );
    std::string line;
    while( std::getline( file, line ) )
    {

        // Do a regex match to extract the calibration data from the line.
        std::string::const_iterator line_start = line.cbegin();
        std::smatch reg_match;

        size_t result;
        if( std::regex_search( line_start, line.cend(), reg_match, numbers ) )
        {
            result = stoll( reg_match[ 1 ] );
            line_start = reg_match.suffix().first;
        }

        std::vector<size_t> inputs;
        while( line.cend() != line_start )
        {
            if( std::regex_search( line_start, line.cend(), reg_match, numbers ) )
            {
                size_t input = stoll( reg_match[ 1 ] );
                inputs.emplace_back( input );

                line_start = reg_match.suffix().first;
            }
        }

        calibration_data.emplace( std::make_pair( result, inputs ) );
    }

    // Step 3: return success or failure.
    return ( 0 != calibration_data.size() );
}

static size_t check_calibration(
    std::map<size_t, std::vector<size_t>>& calibration_data
)
{
    // Two operators are available + and *.
    // Operations always proceed left to right, regardless of real-world
    // operator precedence.
    size_t total_result = 0;
    for( auto& calibration : calibration_data )
    {
        // Brute force it.
        // Try all of the combinations until the correct solution is found or
        // none of the combinations results in the correct answer.

        // There are two operators, can use 0 and 1 to represent the possible
        // operator combinations. For example, treat + = 0 and * = 1.
        // If there are three operator slots there are eight possible
        // operator combinations (2^3):
        // a)  + + + = b000 = 0;  e) * + + = b100 = 4
        // b)  + + * = b001 = 1;  f) * + * = b101 = 5
        // c)  + * + = b010 = 2;  g) * * + = b110 = 6
        // d)  + * * = b011 = 3;  h) * * * = b111 = 7

        // So, loop over the number of possible operators and convert each
        // loop index into the corresponding operations based on the above
        // representation.
        size_t operator_count = static_cast<size_t>( std::pow( 2, calibration.second.size() - 1 ) );
        --operator_count; // Zero-based counting
        for( size_t op_idx = 0; op_idx <= operator_count; ++op_idx )
        {
            size_t op = op_idx;
            std::vector<size_t>::iterator val = calibration.second.begin();
            size_t value = *val;
            ++val;
            while( calibration.second.end() != val )
            {
                if( op & 1 )
                {
                    value *= *val;
                }
                else
                {
                    value += *val;
                }

                ++val;
                op >>= 1;
            }

            if( calibration.first == value )
            {
                // Found a valid operational sequence.
                total_result += calibration.first;
                break;
            }
        }
    }

    return total_result;
}

int main()
{
    std::map<size_t, std::vector<size_t>> calibration_data;
    if( !read_input( calibration_data ) )
    {
        return -1;
    }

    size_t calibration_result = check_calibration( calibration_data );
    std::cout << "Calibration total = " << calibration_result << "\n";
}
