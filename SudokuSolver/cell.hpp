#pragma once

#include "debug.h"
#include "sudoku_types.h"

class cell
{
public:
    cell() = delete; // delete default constructor
    cell( cell& other ) = delete; // delete default copy constructor

    cell( char row, size_t col, size_t box, uint32_t value ) :
        row_idx( row ),     // <-- store as a character (A-I) for easier debug
        col_idx( col + 1 ), // <-- store as 1-based for easier debug
        box_idx( box + 1 ), // <-- store as 1-based for easier debug
        known_value( value ),
        was_initial_value( true )
    {
    }

    cell( char row, size_t col, size_t box ) :
        row_idx( row ),
        col_idx( col + 1 ),
        box_idx( box + 1 ),
        known_value( 0 ),
        was_initial_value( false )
    {
    }

    void init_possible_values( size_t grid_size )
    {
        possible_values.clear();
        for( size_t val = 1; val <= grid_size; ++val )
        {
            possible_values.push_back( static_cast<cell_value_t>( val ) );
        }
    }

    void set_value( cell_value_t value )
    {
        // known_value is either unset or was set at cell creation time.
        // Engine will call set_value with the known value at grid init time.
        ASSERT( 0 == known_value  || value == known_value );
        known_value = value;

        // No other values are possible once the cell value is known.
        possible_values.clear();
    }

    void clear_possible( cell_value_t value )
    {
        possible_values.remove( value );
    }

    void set_single_possible( cell_value_t value )
    {
        possible_values.clear();
        possible_values.push_back( value );
    }

    const size_t get_row( void ) const
    {
        return row_idx - 'A';
    }

    const char get_row_printable( void ) const
    {
        return row_idx;
    }

    const size_t get_column( void ) const
    {
        return col_idx - 1;
    }

    const size_t get_column_printable( void ) const
    {
        return col_idx;
    }

    const size_t get_box( void ) const
    {
        return box_idx - 1;
    }

    const size_t get_box_printable( void ) const
    {
        return box_idx;
    }

    const cell_value_t get_value( void ) const
    {
        return known_value;
    }

    const bool get_was_initial_value( void ) const
    {
        return was_initial_value;
    }

    const std::list<cell_value_t>& get_possible_values( void ) const
    {
        return possible_values;
    }

    void dump_possibles( size_t grid_size, size_t box_size ) const
    {
        std::vector<char> value_text;
        value_text.resize( grid_size + 1 );
        std::fill( value_text.begin(), value_text.end(), ' ' );
        value_text[ grid_size ] = '\0';

        for( cell_value_t possible_value : possible_values )
        {
            value_text[ possible_value - 1 ] = static_cast<char>( possible_value + '0' );
        }
        std::cout << value_text.data();

        if( 0 == ( col_idx % box_size ) )
        {
            std::cout << "|";
        }
        else
        {
            std::cout << ":";
        }
    }

    cell_value_t has_exactly_one_value( void )
    {
        // Check row, column, and block entires for a size of one.
        // Return the cell.
        cell_value_t value = 0;
        if( 1 == possible_values.size() )
        {
            value = possible_values.front();
        }

        return value;
    }

    const bool can_have_value( cell_value_t value ) const
    {
        return ( std::find( possible_values.begin(), possible_values.end(), value ) != possible_values.end() );
    }

    const bool cell_is_pair_eligible( bool is_naked )
    {
        // Naked pair requires exactly two possible values while hidden pair
        // needs at least two possible values.
        bool is_eligible = false;
        if( is_naked )
        {
            is_eligible = ( 2 == possible_values.size() );
        }
        else
        {
            is_eligible = ( 2 <= possible_values.size() );
        }

        return is_eligible;
    }

    static bool cell_sort( std::shared_ptr<cell>& a, std::shared_ptr<cell>& b )
    {
        bool sorts_before =
            ( a.get()->row_idx < b.get()->row_idx ) ||
            ( ( a.get()->row_idx == b.get()->row_idx ) && ( a.get()->col_idx < b.get()->col_idx ) );

        return sorts_before;
    }

private:
    char     row_idx;
    size_t   col_idx;
    size_t   box_idx;

    cell_value_t known_value;
    bool         was_initial_value;

    std::list<cell_value_t> possible_values;
};