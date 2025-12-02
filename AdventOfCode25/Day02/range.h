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
    bool is_invalid_entry( uint64_t val ) const
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

    uint64_t start_;
    uint64_t end_;
};
