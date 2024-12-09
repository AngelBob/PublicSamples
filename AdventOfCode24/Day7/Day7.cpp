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
    std::ifstream file( ".\\Calibrations.txt" );

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
    std::map<size_t, std::vector<size_t>>& calibration_data,
    const size_t num_ops
)
{
    // Two operators are available + and *.
    // Operations always proceed left to right, regardless of real-world
    // operator precedence.
    size_t total_result = 0;
    std::map<size_t, std::vector<size_t>>::iterator cal_it = calibration_data.begin();
    while( calibration_data.end() != cal_it )
    {
        // Brute force it.
        // Try all of the combinations until the correct solution is found or
        // none of the combinations results in the correct answer.

        // There are some number of operators, use 0, 1, 2 ... num_ops - 1
        // to represent the operator operations. For example, if there are
        // three operators, treat '+' = 0, '*' = 1, and || = 2.
        // In this case, if there are two operator slots there are 9 possible
        // operator combinations (3^2):
        // a)  + + = t00 = 0;  c)  * + = t10 = 3;  f)  | + = t20 = 6;
        // b)  + * = t01 = 1;  d)  * * = t11 = 4;  g)  | * = t21 = 7;
        // c)  + | = t02 = 2;  e)  * | = t12 = 5;  h)  | | = t22 = 8;

        // So, loop over the number of possible operators and convert each
        // loop index into the corresponding operations based on the above
        // representation.
        const std::pair<size_t, std::vector<size_t>>& calibration = *cal_it;
        bool is_good = false;

        // First, how many operations are there in this calibration calc?
        size_t cal_op_count = static_cast<size_t>( std::pow( num_ops, calibration.second.size() - 1 ) );
        --cal_op_count; // Zero-based counting on the combinations

        // Loop over each operation combination and convert the index into
        // appropriate operations between each of the values, left to right.
        for( size_t op_combo = 0; op_combo <= cal_op_count; ++op_combo )
        {
            std::vector<size_t>::const_iterator val = calibration.second.begin();
            size_t value = *val; ++val;

            // Need to break the operation combination into it's constituent
            // parts. Start with the largest power of N and work down.
            size_t ops = op_combo;

            // Find the largest power of N that is needed for this combination.
            // It's the number of operation slots minus 1 or the number of
            // values minus 2.
            size_t op_idx_power = calibration.second.size() - 2;

            // Loop over all of the values
            while( calibration.second.end() != val )
            {
                // Base N value for this operation index.
                size_t power_of = static_cast<size_t>( std::pow( num_ops, op_idx_power ) );

                // Translate that number to the corresponding operation.
                // How many "Base Ns" does this value have?
                size_t which_op = ops / power_of;
                switch( which_op )
                {
                case 0:
                    value += *val;
                    break;
                case 1:
                    value *= *val;
                    break;
                case 2:
                    // Need to shift value to the left by ten for each power of
                    // ten in the new value (10^0 is a power of ten...).
                    {
                        size_t new_val = *val;
                        while( new_val )
                        {
                            value *= 10;
                            new_val /= 10;
                        }
                    }
                    value += *val;
                    break;
                }

                // Move to the next value in the list
                ++val;

                // Remove this operation from the operation list and move to
                // the next operation.
                ops -= ( power_of * which_op );
                --op_idx_power;
            }

            if( calibration.first == value )
            {
                // Found a valid operational sequence, stop the search
                is_good = true;
                break;
            }
        }

        if( is_good )
        {
            // Found a valid operational sequence, add the result to the total.
            total_result += calibration.first;

            // Remove this data from the map to avoid checking it again.
            cal_it = calibration_data.erase( cal_it, std::next( cal_it ) );
        }
        else
        {
            ++cal_it;
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

    size_t calibration_result = check_calibration( calibration_data, 2 );
    std::cout << "Calibration total (with 2 operators) = " << calibration_result << "\n";

    calibration_result += check_calibration( calibration_data, 3 );
    std::cout << "Calibration total (with 3 operators) = " << calibration_result << std::endl;
}
