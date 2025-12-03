#pragma once
#include <algorithm>
#include <string>
#include <vector>

class bank
{
public:
    bank() = delete;
    bank( std::string& joltage )
    {
        std::transform( joltage.begin(),
                        joltage.end(),
                        std::back_inserter( m_joltage ),
                        []( char c )
                        {
                            return static_cast<uint8_t>( c - '0' );
                        } );
    }

    inline uint64_t calculate_joltage( size_t digits ) const
    {
        // Find the largest 'digits' values in order in a vector of numbers.
        // For example, in the vector { 1, 9, 3, 8, 4, 7 } and digits = 3,
        // the result is { 9, 8, 7 }.
        std::vector<uint8_t> value;
        value.reserve( digits );

        // Start at the beginning of the vector and search for the largest
        // digit, stopping when there are only 'digits' entries left to search.
        size_t d_idx = 0;
        while( digits )
        {
            const size_t max_idx = m_joltage.size() - digits;

            // If the max index is equal to the current search index
            // then there is no need to continue the search, the rest
            // of the digits must be used.
            if( max_idx == d_idx )
            {
                std::copy( m_joltage.begin() + d_idx,
                           m_joltage.end(),
                           std::back_inserter( value ) );
                break;
            }

            // Find the largest digit in this range.
            size_t n_idx = d_idx + 1;
            while( n_idx <= max_idx )
            {
                if( m_joltage[ n_idx ] > m_joltage[ d_idx ] )
                {
                    d_idx = n_idx;
                    if( m_joltage[ d_idx ] == 9 )
                    {
                        // 9 is the largest possible digit; stop searching.
                        break;
                    }
                }
                ++n_idx;
            }

            // Store this digit and continue.
            value.push_back( m_joltage[ d_idx ] );
            ++d_idx;
            --digits;
        }

        // Convert the stored digits into a number.
        uint64_t sum = 0;
        uint64_t mult = 1;
        while( value.size() )
        {
            sum += ( value.back() * mult );
            mult *= 10;
            value.pop_back();
        }

        // Return the number.
        return sum;
    }

private:
    std::vector<uint8_t> m_joltage;
};