// MostSignificantBit.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Efficient method of identifying the most significant bit of an integer type.
// Returns -1 if no bits are set in the input value.

#include <iostream>

using ULONG = unsigned long;
using ULONG64 = unsigned long long;

// Binary search for most significant bit
template<typename T>
ULONG MsbSet( T Value )
{
    // short-circuit on zero value
    if( !Value ) return -1;

    ULONG msb = ( sizeof( T ) * CHAR_BIT ) / 2;

    ULONG half_msb = msb;
    T half_mask = ( -1 );

    while( half_msb )
    {
        T mask = half_mask << msb;

        if( Value & mask )
        {
            half_msb >>= 1;
            half_mask >>= half_msb;

            msb += half_msb;
        }
        else
        {
            msb -= half_msb;
        }
    }

    return msb;
}

int main()
{
    const ULONG64 TestLongs[] =
    {
        0x8000000000000000,
        0x0001000000000000,
        0x0000800000000000,
        0x0000000100000000,
        0x0000000080000000,
        0x0000000000010000,
        0x0000000000008000,
        0x0000000000000001,
        0x0000800200803040,
        0x0000000000000000,
    };

    for( int i = 0; i < sizeof( TestLongs ) / sizeof( TestLongs[ 0 ] ); ++i )
    {
        std::cout << "0x" << std::hex << TestLongs[ i ] << " = " << std::dec << MsbSet( TestLongs[ i ] ) << std::endl;
    }

    const ULONG TestShorts[] =
    {
        0x80000000,
        0x00010000,
        0x00008000,
        0x00000001,
        0x00803040,
        0x00000000,
    };

    for( int i = 0; i < sizeof( TestShorts ) / sizeof( TestShorts[ 0 ] ); ++i )
    {
        std::cout << "0x" << std::hex << TestShorts[ i ] << " = " << std::dec << MsbSet( TestShorts[ i ] ) << std::endl;
    }
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
