// Day06.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <vector>

static std::vector<std::string> split( const std::string& str )
{
    std::stringstream ss( str );
    std::vector<std::string> result;
    for( std::string token; ss >> token; )
    {
        result.push_back( token );
    }

    return result;
}

static bool read_input(
    std::vector<std::string>& input1,
    std::list<char>& input2
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( ".\\Data\\Input_test.txt" );

    // Step 2: read the lines and create the input data.
    bool is_operation = false;
    size_t token_count = 0;
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        char test = line[ 0 ];
        if( '*' == test || '+' == test )
        {
            // operator line.
            is_operation = true;
        }

        if( !is_operation )
        {
            input1.emplace_back( std::move( line ) );
        }
        else
        {
            const std::vector<std::string> tokens = split( line );
            for( const auto& op : tokens )
            {
                input2.emplace_back( op[ 0 ] );
            }
        }
    }

    // Step 3: return success or failure.
    return true;
}

static uint64_t get_column_width(
    const std::vector<std::string>& input
)
{
    size_t max_col = 0;
    for( auto& row : input )
    {
        size_t col = 0;
        size_t i;
        for( i = col; i < row.length() && row[ i ] == ' '; ++i )
        {
            ;
        }
        col = i;

        for( i = col; i < row.length() && row[ i ] != ' '; ++i )
        {
            ;
        }
        col = i;

        if( col > max_col )
        {
            max_col = col;
        }
    }

    return max_col;
}

static void process_horizontal_rows(
    size_t max_col,
    std::vector<std::string>& input,
    std::vector<std::list<uint64_t>>& transposed
)
{
    // Numbers are in the first max_col columns of each row.
    std::list<uint64_t> new_row;
    for( auto& row : input )
    {
        uint64_t value = std::atoll( row.c_str() );
        new_row.emplace_back( value );
        row.erase( 0, max_col + 1 );
    }

    transposed.emplace_back( std::move( new_row ) );
}

void transpose(
    std::vector<std::string>& input,
    std::vector<std::list<uint64_t>>& transposed
)
{
    while( input[ 0 ].size() )
    {
        // Find the max width of this column of numbers.
        size_t max_col = get_column_width( input );

        // Extract the values one column at a time.
        process_horizontal_rows( max_col, input, transposed );
    }
}

static uint64_t do_calculation(
    std::list<char>& ops,
    std::vector<std::list<uint64_t>>& numbers
)
{
    uint64_t sum = 0;
    while( ops.size() )
    {
        char op = ops.back();
        ops.pop_back();

        std::list<uint64_t> row = numbers.back();
        numbers.pop_back();

        uint64_t result = 0;
        if( '*' == op )
        {
            result = 1;
            for( const auto& value : row )
            {
                result *= value;
            }
        }
        else if( '+' == op )
        {
            for( const auto& value : row )
            {
                result += value;
            }
        }

        sum += result;
    }

    return sum;
}

int main()
{
    std::vector<std::string> input1;
    std::list<char> input2;
    if( !read_input( input1, input2 ) )
    {
        return -1;
    }

    // Process the input data.
    std::vector<std::list<uint64_t>> numbers;
    transpose( input1, numbers );

    // Do the calculations.
    uint64_t sum = do_calculation( input2, numbers );
    std::cout << "The sum is: " << sum << std::endl;
}
