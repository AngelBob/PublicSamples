#pragma once

#include "directions.h"

class puzzle_map
{
    using maprow_t = std::vector<char>;
    using map_t = std::vector<maprow_t>;

public:
    puzzle_map()
        : x_max( 0 )
        , y_max( 0 )
    {
    }

    bool add_row( const std::string& row )
    {
        // Convert the string into a vector of chars
        maprow_t array( row.begin(), row.end() );

        // Push the new row
        m_map.emplace_back( std::move( array ) );

        y_max = m_map.size();
        if( y_max > 1 )
        {
            return ( x_max == m_map[ y_max - 1 ].size() );
        }

        x_max = m_map[ 0 ].size();
        return true;
    }

    size_t count_accessible_rolls( void )
    {
        // Count the number of accessible rolls. Rolls are accessible iff there
        // are fewer than 4 rolls in the adjacent 8 cells.
        size_t accessible = 0;
        for( size_t y = 0; y < y_max; ++y )
        {
            for( size_t x = 0; x < x_max; ++x )
            {
                accessible += check_grid( x, y );
            }
        }

        return accessible;
    }

private:
    size_t check_grid( const size_t x, const size_t y )
    {
        size_t accessible = 0;
        if( is_roll_accessible( x, y ) )
        {
            ++accessible;
        }

        return accessible;
    }

    bool is_roll_accessible( const size_t x, const size_t y ) const
    {
        if( x >= x_max ||
            y >= y_max ||
            m_map[ y ][ x ] != '@' )
        {
            return false;
        }

        size_t neighbors = 0;

        compass c;
        compass::iterator it = c.begin();
        while( it != c.end() )
        {
            const auto [x_off, y_off] = compass::get_xy_offset( *it );
            if( ( x + x_off ) >= x_max ||
                ( y + y_off ) >= y_max )
            {
                ++it;
                continue;
            }

            if( m_map[ y + y_off ][ x + x_off ] == '@' )
            {
                ++neighbors;
            }
            ++it;
        }

        return ( neighbors < 4 );
    }

    size_t x_max;
    size_t y_max;

    map_t m_map;
};