#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static const unsigned int target = 5;

inline void test_case_0( std::vector<unsigned int>& nums, unsigned int& total )
{
    while( nums.size() )
    {
        // This is O(n^2) because count_if checks every element.
        const unsigned int first = nums.back();
        nums.pop_back();

        // Assume that there are values <= target in the array...
        // Unsigned declaration already defines no values less than zero, so
        // it's okay to skip numbers larger than target.
        if( first > target )
        {
            // > target allows zeros in the list
            continue;
        }

        const unsigned int find = target - first;
        total += std::count_if( nums.begin(),
                                nums.end(),
                                [ find ](const int& a){ return a == find; } );
    }
}

inline void test_case_1( std::vector<unsigned int>& nums, unsigned int& total )
{
    while( nums.size() )
    {
        // This is still 0(n^2), but worse than the solution above because
        // there are two instances of the count_if construct, each of which is
        // O(n^2).
        const unsigned int first = nums.back();

        // Assume that there are values <= target in the array...
        // Unsigned declaration already defines no values less than zero, so
        // it's okay to skip numbers larger than target.
        if( first > target )
        {
            // > target allows zeros in the list.
            nums.pop_back();
            continue;
        }

        unsigned int x_count = std::count_if(
            nums.begin(),
            nums.end(),
            [ first ](const unsigned int& a){ return a == first; } );

        if( first + first != target )
        {
            const unsigned int find = target - first;
            unsigned int y_count = std::count_if(
                nums.begin(),
                nums.end(),
                [ find ](const int& a){ return a == find; } );

            total += x_count * y_count;
        }
        else
        {
            // Special case for when x + y == target
            total += ( x_count * ( x_count - 1 ) ) / 2;
        }

        // With -O3 flag this should be faster than looping on pop_back()
        nums.erase( nums.end() - x_count, nums.end() );
    }
}

inline void test_case_2( std::vector<unsigned int>& nums, unsigned int& total )
{
    // Use the fact that the vector is sorted to avoid checking all elements.

    // I don't know how to calculate the time complexity of this one, but the
    // perf runs show that this solution is worse than the previous two because
    // of the three binary search loops.
    auto big_last = nums.end();
    while( nums.begin() != big_last )
    {
        big_last--;

        // Assume that there are values <= target in the array...
        // Unsigned declaration already defines no values less than zero, so
        // it's okay to skip numbers larger than target.
        if( *big_last > target )
        {
            // > target allows zeros in the list.
            continue;
        }

        // Binary search #1
        auto big_first = std::lower_bound( nums.begin(), big_last, *big_last );
        unsigned int x_count = std::distance( big_first, big_last ) + 1;

        if( ( ( *big_last ) * 2 ) != target )
        {
            const unsigned int find = target - *big_last;

            // Binary search #2
            unsigned int y_count = 0;
            auto small_first = std::lower_bound( nums.begin(), big_first, find );
            if( nums.end() != small_first && find == *small_first )
            {
                // Binary search #3
                auto small_last = std::upper_bound( small_first, big_first, find );
                unsigned int y_count = std::distance( small_first, small_last );

                total += x_count * y_count;
            }
        }
        else
        {
            // Special case for when x + y == target
            total += ( x_count * ( x_count - 1 ) ) / 2;
        }

        big_last = big_first;
    }
}

inline void test_case_3( std::vector<unsigned int>& nums, unsigned int& total )
{
    // Try a simple two pointer solution.
    // This is definitely O(n) and perf runs show it to be much faster than
    // the other three solutions.
    unsigned int* front = nums.data();
    unsigned int* back = nums.data() + nums.size() - 1;

    while( front < back )
    {
        // Assume that there are values <= target in the array...
        while( *back > target )
        {
            --back;
        }

        unsigned int x_count = 1;
        while( *back == *( back - 1 ) )
        {
            ++x_count;
            --back;
        }

        if( ( ( *back ) * 2 ) != target )
        {
            while( *front + *back < target )
            {
                ++front;
            }

            unsigned int y_count = 0;
            while( *front + *back == target )
            {
                ++y_count;
                ++front;
            }

            total += x_count * y_count;
        }
        else
        {
            // Special case for when x + y == target
            total += ( x_count * ( x_count - 1 ) ) / 2;
        }

        --back;
    }
}

static const std::vector<unsigned int> nums = { 1, 1, 2, 2, 3, 3, 4, 5 };
static const int repititions = 1000000;

std::array<std::vector<unsigned int>, repititions> test_cases;

extern "C" void count_pairs( unsigned int test_type )
{
    // Copy the vector into an array and sequentially count the pairs in each.
    // Eliminates the need to copy the vector on each iteration, thus taking
    // the copy out of the perf hot path.
    for( int i = 0; i < repititions; ++i )
    {
        test_cases[ i ].resize( nums.size() );
        std::copy( nums.begin(), nums.end(), test_cases[ i ].begin() );
    }

    // Do some perf measurements.
    // The functions get in-lined in an optimized build so there's no call
    // overhead; the only non-algorithmic overhead is the loop counter logic.
    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency( &freq );
    QueryPerformanceCounter( &t0 );

    unsigned int total = 0;
    switch( test_type )
    {
    case 0:
        for( int i = 0; i < repititions; ++i )
        {
            total = 0;
            test_case_0( test_cases[ i ], total );
        }
        break;

    case 1:
        for( int i = 0; i < repititions; ++i )
        {
            total = 0;
            test_case_1( test_cases[ i ], total );
        }
        break;

    case 2:
        for( int i = 0; i < repititions; ++i )
        {
            total = 0;
            test_case_2( test_cases[ i ], total );
        }
        break;

    case 3:
        for( int i = 0; i < repititions; ++i )
        {
            total = 0;
            test_case_3( test_cases[ i ], total );
        }
        break;
    }

    QueryPerformanceCounter( &t1 );

    std::cout << "usec: " << ( ( t1.QuadPart - t0.QuadPart ) * 1000000LL ) / freq.QuadPart;
    std::cout << " Total pairs that sum: " << total << "\n";
}
