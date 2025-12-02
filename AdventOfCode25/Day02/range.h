#pragma once

#include <string>

class range_iterator
{
public:
    range_iterator( uint64_t value ) : value_( value )
    {
    }
    uint64_t operator*() const
    {
        return value_;
    }
    range_iterator& operator++()
    {
        ++value_;
        return *this;
    }
    bool operator!=( const range_iterator& other ) const
    {
        return value_ != other.value_;
    }

private:
    uint64_t value_;
};

template<bool count_any>
class range
{
public:
    using iterator = range_iterator;

    range( void ) = delete;

    range( uint64_t start, uint64_t end )
        : start_( start )
        , end_( end )
    {
    }

    iterator begin() const
    {
        return iterator( start_ );
    }
    iterator end() const
    {
        return iterator( end_ + 1 );
    }

    uint64_t total_invalid_entries() const
    {
        // Search the range_ts for invalid entries and total them.
        uint64_t sum = 0;
        iterator iter = begin();
        while( iter != end() )
        {
            if( is_invalid_entry( *iter ) )
            {
                sum += *iter;
            }
            ++iter;
        }

        return sum;
    }

private:
    inline bool is_invalid_entry( uint64_t val ) const
    {
        if constexpr( !count_any )
        {
            return count_repeated_twice( val );
        }
        else
        {
            return count_repeated_any( val );
        }
    }

    inline bool count_repeated_twice( uint64_t val ) const
    {
        // Check if the entry is made up of a set of numbers repeated twice.
        // (e.g., 55, 6464, 123123).

        // Convert the entry to a string for easier manipulation.
        std::string val_str = std::to_string( val );
        size_t length = val_str.length();

        // An entry can only be invalid if it has an even number of digits.
        if( length % 2 != 0 )
        {
            return false;
        }

        // Split the string into two halves and compare them.
        std::string first_half = val_str.substr( 0, length / 2 );
        std::string second_half = val_str.substr( length / 2, length / 2 );

        return ( first_half == second_half );
    }

    inline bool count_repeated_any( uint64_t val ) const
    {
        // Check if the entry is made up of a set of numbers repeated any
        // number of times (e.g., 55, 999, 6464, 123123, 565656 ).

        // Convert the entry to a string for easier manipulation.
        std::string val_str = std::to_string( val );
        size_t length = val_str.length();

        // In this case odd length entries can also be invalid (e.g., 33333).
        // Check all possible lengths for a repeated set.
        size_t max_set_length = length / 2;

        // Split the string and compare the parts.
        while( 0 < max_set_length )
        {
            std::string first_set = val_str.substr( 0, max_set_length );
            size_t offset = max_set_length;

            bool found_repeat = true;
            while( val_str.length() > offset )
            {
                std::string next_part = val_str.substr( offset, max_set_length );
                if( next_part != first_set )
                {
                    found_repeat = false;
                    break;
                }
                offset += max_set_length;
            }

            if( found_repeat )
            {
                return true;
            }

            --max_set_length;
        }

        return false;
    }

    uint64_t start_;
    uint64_t end_;
};
