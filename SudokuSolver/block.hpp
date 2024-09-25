#pragma once

#include "debug.h"
#include "sudoku_types.h"
#include "cell.hpp"

class block
{
public:
    block( void ) = delete;
    block( block& other ) = delete;

    block( size_t idx, std::array<size_t, 4> extents, BLOCK_TYPE type ) :
        block_index( idx ),
        block_type( type ),
        block_extents( extents )
    {
    }

    void init_needed_values( size_t grid_size )
    {
        for( size_t val = 1; val <= grid_size; ++val )
        {
            needed_values.push_back( static_cast<uint32_t>( val ) );
        }
    }

    void emplace_back( std::shared_ptr<cell>& cell )
    {
        block_cells.emplace_back( cell );
    }

    void unset_needed( cell_value_t value )
    {
        // Block no longer needs this value.
        needed_values.remove( value );
    }

    const size_t get_index( void ) const
    {
        return block_index;
    }

    const BLOCK_TYPE get_type( void ) const
    {
        return block_type;
    }

    cell_array_t& get_cells( void )
    {
        return block_cells;
    }

    cell* get_cell( size_t cell_idx )
    {
        return block_cells[ cell_idx ].get();
    }

    const std::list<cell_value_t>& get_needed_values( void ) const
    {
        return needed_values;
    }

    void print_row( size_t box_size, size_t leader_width, size_t col_width ) const
    {
        std::cout << std::setw( leader_width ) << std::left << static_cast<char>( block_index + 'A' ) << "|";

        size_t col_idx = 0;
        for( const std::shared_ptr<cell>& row_cell : block_cells )
        {
            row_cell->print_value( col_width );

            if( 0 == ( ++col_idx % box_size ) )
            {
                std::cout << " |";
            }
        }

        std::cout << std::endl;
    }

    void dump_possibles( size_t grid_size, size_t box_size, size_t leader_width ) const
    {
        std::cout << std::setw( leader_width ) << std::left << static_cast< char >( block_index + 'A' ) << "|";

        size_t col_idx = 0;
        for( const std::shared_ptr<cell>& row_cell : block_cells )
        {
            row_cell->dump_possibles( grid_size, box_size );

            if( 0 == ( ++col_idx % box_size ) )
            {
                std::cout << "|";
            }
            else
            {
                std::cout << ":";
            }
        }

        std::cout << std::endl;
    }

    const std::array<size_t, 4>& get_extents( void ) const
    {
        return block_extents;
    }

    cell_value_t check_block_cell_for_unique_possible( cell* cur_cell )
    {
        std::list<cell_value_t> unique_values = cur_cell->get_possible_values();

        cell_array_t::iterator compare_cell = block_cells.begin();
        while( compare_cell != block_cells.end() )
        {
            if( ( cur_cell->get_row() == ( *compare_cell )->get_row() ) &&
                ( cur_cell->get_column() == ( *compare_cell )->get_column() ) )
            {
                // Skip comparison to self.
                ++compare_cell;
                continue;
            }

            std::list<cell_value_t> difference;
            std::set_difference(
                unique_values.begin(),
                unique_values.end(),
                ( *compare_cell )->get_possible_values().begin(),
                ( *compare_cell )->get_possible_values().end(),
                std::back_inserter( difference ) );

            unique_values = difference;

            if( unique_values.empty() )
            {
                break;
            }

            ++compare_cell;
        }

        if( 1 == unique_values.size() )
        {
            return unique_values.front();
        }

        return 0;
    }

    bool get_naked_hidden_pairs(
        std::vector<std::pair<cell*, cell*>>& pairs,
        bool is_naked )
    {
        // Find cells that are eligible for a naked/hidden pair test.
        // That means a pair of cells that have a pair of possible values that
        // are only common between those two cells. For a "naked" pair the
        // cells have only those to shared values, for a "hidden" pair the
        // cells may have other possible values, but are the only two cells
        // with the two shared possible values.
        std::vector<std::tuple<cell*, cell*, cell_value_t, cell_value_t>> pair_eligible;
        cell_array_t::iterator cur_cell = block_cells.begin();
        while( cur_cell != block_cells.end() )
        {
            if( !( *cur_cell )->cell_is_pair_eligible( is_naked ) )
            {
                ++cur_cell;
                continue;
            }

            // Cell is eligible, compare it against all of the other cells in
            // the block to see if there's a match.
            cell_array_t::iterator next_cell;
            next_cell = cur_cell; ++next_cell;
            while( next_cell != block_cells.end() )
            {
                if( !( *next_cell )->cell_is_pair_eligible( is_naked ) )
                {
                    ++next_cell;
                    continue;
                }

                // Current cell and next cell are pair eligible, see if they
                // make a pair.
                std::vector<cell_value_t> common_values;
                std::set_intersection(
                    ( *cur_cell )->get_possible_values().begin(),
                    ( *cur_cell )->get_possible_values().end(),
                    ( *next_cell )->get_possible_values().begin(),
                    ( *next_cell )->get_possible_values().end(),
                    std::back_inserter( common_values )
                );

                if( 2 == common_values.size() )
                {
                    // Cells have two common values, add them to the pair list.
                    pair_eligible.emplace_back( std::make_tuple(
                        ( *cur_cell ).get(),
                        ( *next_cell ).get(),
                        common_values[ 0 ],
                        common_values[ 1 ] )
                    );
                }

                ++next_cell;
            }

            ++cur_cell;
        }

        // For each pair validate that no other cells in the block can have
        // either of the shared values.
        std::vector<std::tuple<cell*, cell*, cell_value_t, cell_value_t>>::iterator cur_tuple = pair_eligible.begin();
        while( cur_tuple != pair_eligible.end() )
        {
            bool do_advance = true;

            cell* cell1 = std::get<0>( *cur_tuple );
            cell* cell2 = std::get<1>( *cur_tuple );

            cell_value_t value1 = std::get<2>( *cur_tuple );
            cell_value_t value2 = std::get<3>( *cur_tuple );

            for( std::shared_ptr<cell>& cur_cell : block_cells )
            {
                if( cur_cell.get() == cell1 || cur_cell.get() == cell2 )
                {
                    continue;
                }

                if( cur_cell->can_have_value( value1 ) || cur_cell->can_have_value( value2 ) )
                {
                    // Values not unique, delete pair from the eligible list.
                    cur_tuple = pair_eligible.erase( cur_tuple );
                    do_advance = false;
                    break;
                }
            }

            if( do_advance )
            {
                ++cur_tuple;
            }
        }
 
        // If there are any pairs left, move them into the output.
        for( std::tuple<cell*, cell*, cell_value_t, cell_value_t>& tuple_val : pair_eligible )
        {
            pairs.emplace_back( std::make_pair( std::get<0>( tuple_val ), std::get<1>( tuple_val ) ) );
        }

        return !pairs.empty();
    }

private:
    void print_row_leader( size_t leader_width )
    {
        ASSERT( BLOCK_TYPE::BLOCK_TYPE_ROW == block_type );

        std::cout << std::setw( leader_width ) << std::left << static_cast<char>( block_index + 'A' ) << "|";
    }

    BLOCK_TYPE   block_type;
    size_t       block_index;

    std::array<size_t, 4> block_extents;

    std::list<cell_value_t> needed_values;

    cell_array_t block_cells;
};