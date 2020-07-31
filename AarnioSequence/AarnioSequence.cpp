// AarnioSequence.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Purpose:  Find integral numbers that evenly divide by their reversed selves, e.g., 8712 & 2178

#include <iostream>
#include <math.h>
#include <string>

int get_exponent( const unsigned int value )
{
    std::string value_str = std::to_string( value );
    return static_cast<int>( value_str.length() - 1 );
}

bool low_number_skip( unsigned int& value )
{
    // Numbers that have units digit larger than 2x leading digit should be skipped.
    // Dividing smaller numbers by bigger numbers gives a 1/whole number result
    // which is not useful for this search.
    int exponent = get_exponent( value );
    int base_value = static_cast<int>( pow( 10, exponent ) );
    int leading_digit = static_cast< int >( value / base_value );

    int trailing_digit = value % 10;

    return ( leading_digit < trailing_digit * 2 );
}

int reverse_number( unsigned int number )
{
    int reversed_num = 0;

    int exponent_in = get_exponent( number );
    int exponent_out = 0;

    do
    {
        // Find each digit
        int base = static_cast<int>( pow( 10, exponent_in-- ) );
        int digit = static_cast< int >( number / base );
        if( 0 != digit )
        {
            // Remove the digit from the starting number
            number -= static_cast<int>( digit * base );

            // Put the digit into the reverse order position
            digit *= static_cast< int >( pow( 10, exponent_out ) );

            // Add the reversed digit to the output value
            reversed_num += digit;
        }
        ++exponent_out;
    } while( number );

    return reversed_num;
}

int main()
{
    unsigned int value = 1000;
    while( value < 1000000 )
    {
        // Numbers that end in zero don't count, nor do numbers whose MSD is less than 2x the LSD
        if( !( value % 10 ) || low_number_skip( value ) )
        {
            value++;
            continue;
        }

        // Reverse the number
        int divisor = reverse_number( value );

        // Palindromes don't count
        if( divisor == value )
        {
            value++;
            continue;
        }

        // Look for an integer division result
        float float_result = static_cast<float>( value ) / static_cast<float>( divisor );
        float   int_result = static_cast<float>( value / divisor );
        if( float_result == static_cast<float>( int_result ) )
        {
            // Yippee, integer result, pring it out.
            std::cout << value << " & " << divisor << " ( =" << int_result << " )\n";
        }

        // Move to the next number
        value++;
    }

    std::cout << "Done!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
