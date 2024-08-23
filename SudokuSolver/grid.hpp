#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <tuple>
#include <utility>

#include "debug.h"
#include "sudoku_types.h"
#include "block.hpp"
#include "cell.hpp"

class grid
{
public:
    grid( void ) :
        grid_size( 4 ), // <-- 4x4 is the minimum supported grid size
        box_size( 2 )
    {
    }

    void set_grid_size(
        size_t size
    )
    {
        // Check if the grid size is usable - only perfect squares are allowed.
        size_t root = static_cast<size_t>( std::sqrt( size ) );
        if( ( root * root ) != size )
        {
            throw( std::exception( "Invalid grid size!" ) );
        }

        // Grid size ok, store the grid and box sizes.
        grid_size = size;
        box_size = root;

        // Create the blocks - cells grouped into rows, columns and boxes.
        create_blocks( grid_size, box_size );
    }

    void set_grid_parameters( PROGRAM_ARGS parsed_parameters )
    {
        parameters = parsed_parameters;
    }

    void add_cell(
        std::shared_ptr<cell>& new_cell
    )
    {
        // Add a single cell to the three blocks to which it belongs.
        size_t row = new_cell->get_row();
        size_t col = new_cell->get_column();
        size_t box = ( ( row / box_size ) * box_size ) + ( col / box_size );

        assert( row < block_arrays[BLOCK_TYPE::BLOCK_TYPE_ROW].size() );
        block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ][ row ]->emplace_back( new_cell );

        assert( col < block_arrays[ BLOCK_TYPE::BLOCK_TYPE_COL ].size() );
        block_arrays[ BLOCK_TYPE::BLOCK_TYPE_COL ][ col ]->emplace_back( new_cell );

        assert( box < block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ].size() );
        block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ][ box ]->emplace_back( new_cell );
    }

    void initialize_grid( void )
    {
        // Set the possible values on each cell.
        for( std::shared_ptr<block>& row : block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ] )
        {
            assert( BLOCK_TYPE::BLOCK_TYPE_ROW == row->get_type() );

            cell_array_t& cells = row->get_cells();
            for( std::shared_ptr<cell>& cell : cells )
            {
                assert( cell->get_row() == row->get_index() );
                cell->init_possible_values( grid_size );
            }
        }

        // Set the needed values for each row, column and box.
        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            for( std::shared_ptr<block>& block_array : block_arrays[ block_type ] )
            {
                assert( static_cast<BLOCK_TYPE>( block_type ) == block_array->get_type() );

                // Set the needed values for each block.
                block_array->init_needed_values( grid_size );
            }
        }
    }

    const size_t get_grid_size( void ) const
    {
        return grid_size;
    }

    const size_t get_box_size( void ) const
    {
        return box_size;
    }

    const block_array_t& get_rows( void ) const
    {
        return block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ];
    }

    void set_value(
        cell* solved_cell,
        cell_value_t value
    )
    {
        pre_set_verify( solved_cell, value );

        // Set the known value for the given cell.
        // The cell will clear the possible values list at this time.
        solved_cell->set_value( value );

        // Remove this value from available and needed values of each
        // associated row, column and box in the grid.
        clear_value( solved_cell, value );

        post_set_verify( solved_cell, value );
    }

    void update_possibles( std::vector<std::pair<cell*, cell_value_t>>& solvable_cells )
    {
        // Look for opportunities to adjust the allowable values per cell.
        // Examine each block looking for ways to eliminate possible values.

        // Goal is to reduce the number of possible values in a block's
        // cells in an attempt to create a cell with only a single
        // possible value.
        bool strategies_exhausted = false;
        size_t search_type = SEARCH_STRATEGY::STRATEGY_HIDDEN_SINGLE;

        if( parameters.annotations.test( ANNOTATION_BITS::ANNOTATIONS_BASIC ) )
        {
            std::cout << "Grid update results:\n";
        }

        std::stringstream annotations;
        annotations << "\t";

        while( !have_single_possible( solvable_cells, annotations ) &&
               !strategies_exhausted
             )
        {
            bool was_successful = false;

            switch( search_type )
            {
            case SEARCH_STRATEGY::STRATEGY_HIDDEN_SINGLE:
                was_successful = resolve_hidden_single( annotations );
                break;

            case SEARCH_STRATEGY::STRATEGY_POINTING_PAIR:
                // Two cells that live at the intersection of a box and row or
                // column and have a set of shared possible values that are not
                // possible in the rest of the box. Since the value must be in
                // one of the two cells in the box, it can be removed from the
                // intersecting row or column.
                was_successful = resolve_pointing_pairs( annotations );
                break;

            case SEARCH_STRATEGY::STRATEGY_NAKED_PAIR:
                // Two cells in a row, column, or box each contain the same two
                // possible values, and only those two possibles. Those values
                // can be eliminated from the other cells in the block.
                was_successful = resolve_naked_pairs( annotations );
                break;

            case SEARCH_STRATEGY::STRATEGY_HIDDEN_PAIR:
                // Two cells in a row, column, or box each contain the same two
                // possible values, and none of the other cells in the block
                // contain either of those values. The two matching cells can
                // remove all possibles except the two matching.
                was_successful = resolve_hidden_pairs( annotations );
                break;

            case SEARCH_STRATEGY::STRATEGY_END:
            default:
                strategies_exhausted = true;
                break;
            }

            if( parameters.annotations.test( ANNOTATION_BITS::ANNOTATIONS_BASIC ) )
            {
                if( 2 < annotations.str().length() )
                {
                    // Drop the trailing tab & new line and replace with flushing
                    // std::endl.
                    std::string output = annotations.str();
                    output.pop_back();
                    output.pop_back();

                    std::cout << output << std::endl;
                    annotations.clear();
                    annotations.str( "" );
                    annotations << "\t";
                }
            }

            if( was_successful )
            {
                // Something worked, restart the search strategies.
                search_type = SEARCH_STRATEGY::STRATEGY_HIDDEN_SINGLE;
            }
            else
            {
                // Last strategy failed, try the next one.
                ++search_type;
            }
        }

        if( parameters.annotations.test( ANNOTATION_BITS::ANNOTATIONS_BASIC ) )
        {
            if( annotations.str().length() )
            {
                std::cout << annotations.str() << std::endl;
            }
        }
    }
 
    void validate_cells( void )
    {
        // Do some checks on the cell and block data.
        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            for( std::shared_ptr<block>& block_array : block_arrays[ block_type ] )
            {
                std::list<cell_value_t> block_needed = block_array->get_needed_values();
                std::list<cell_value_t> block_values;
                for( std::shared_ptr<cell>& block_cell : block_array->get_cells() )
                {
                    cell_value_t cell_value = block_cell->get_value();
                    if( 0 != cell_value )
                    {
                        // Cell is solved:
                        // 1) Verify it's value is not in the block needed list.
                        ASSERT( block_needed.end() == std::find( block_needed.begin(),
                                                                 block_needed.end(),
                                                                 cell_value )
                        );

                        // 2) Verify it's value doesn't already appear in the
                        //    block's solved values list
                        ASSERT( block_values.end() == std::find( block_values.begin(),
                                                                 block_values.end(),
                                                                 cell_value )
                        );

                        block_values.push_back( cell_value );
                    }
                    else
                    {
                        // Cell is unsolved, verify it's possible values are
                        // all in the block's needed list.
                        std::list<cell_value_t> cell_possibles = block_cell->get_possible_values();

                        std::list<cell_value_t> intersection;
                        std::set_intersection(
                            cell_possibles.begin(),
                            cell_possibles.end(),
                            block_needed.begin(),
                            block_needed.end(),
                            std::back_inserter( intersection )
                        );

                        ASSERT( intersection.size() == cell_possibles.size() );
                    }
                }
            }
        }
    }

    void dump_possibles( void )
    {
        if( parameters.annotations.test( ANNOTATION_BITS::ANNOTATIONS_EXTENDED ) )
        {
            size_t row_width = 0;
            size_t row_divisor = grid_size / 10;
            do
            {
                ++row_width;
                row_divisor /= 10;
            } while( row_divisor );

            size_t col_width = grid_size + row_width;

            // Print a blank line and a leader
            print_possibles_grid_header( row_width, col_width );

            size_t row_index = 0;
            for( std::shared_ptr<block>& block_array : block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ] )
            {
                if( 0 == ( row_index++ % box_size ) )
                {
                    print_possibles_grid_divider( row_width, col_width );
                }

                block_array->dump_possibles( grid_size, box_size );
            }

            print_possibles_grid_divider( row_width, col_width );

            std::cout << std::endl;
        }
    }

private:
    void create_blocks(
        size_t grid_size,
        size_t box_size
    )
    {
        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            if( grid_size == block_arrays[ block_type ].size() )
            {
                continue;
            }

            for( size_t block_index = 0; block_index < grid_size; ++block_index )
            {
                std::array<size_t, 4> extents;
                if( BLOCK_TYPE::BLOCK_TYPE_ROW == block_type )
                {
                    extents = { block_index, block_index + 1, 0, grid_size };
                }
                else if( BLOCK_TYPE::BLOCK_TYPE_COL == block_type )
                {
                    extents = { 0, grid_size, block_index, block_index + 1 };
                }
                else
                {
                    size_t start_row = ( block_index / box_size ) * box_size;
                    size_t end_row = start_row + box_size;

                    size_t start_col = ( block_index * box_size ) % grid_size;
                    size_t end_col = start_col + box_size;

                    extents = { start_row, end_row, start_col, end_col };
                }

                block_arrays[ block_type ].emplace_back(
                    std::make_shared<block>( block_index, extents, static_cast<BLOCK_TYPE>( block_type ) )
                );
            }
        }
    }

    bool have_single_possible(
        std::vector<std::pair<cell*, cell_value_t>>& solvable_cells,
        std::stringstream& annotations
    )
    {
        // Search the grid for cells that can only have one possible value.
        block_array_t& block_array = block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ];
        for( size_t block_index = 0; block_index < grid_size; ++block_index )
        {
            std::shared_ptr<block>& block_ptr = block_array[ block_index ];
            cell_array_t::iterator cell_it = block_ptr->get_cells().begin();

            while( cell_it != block_ptr->get_cells().end() )
            {
                // If the cell has exactly one possible value, add it to the
                // solvable cells list and continue.
                if( ( *cell_it )->has_exactly_one_value() )
                {
                    cell_value_t solved_value = ( *cell_it )->get_possible_values().front();
                    solvable_cells.emplace_back( std::make_pair( ( *cell_it ).get(), solved_value ) );
                }

                ++cell_it;
            }
        }

        if( !solvable_cells.empty() )
        {
            annotations << "Found solvable cell";
            if( 1 < solvable_cells.size() )
            {
                annotations << "s";
            }
            annotations << " @ ";

            for( std::pair<cell*, cell_value_t> solvable_cell : solvable_cells )
            {
                annotations <<
                    solvable_cell.first->get_row_printable() <<
                    solvable_cell.first->get_column_printable() << " ";
            }
        }

        // Return true if there are any solvable cells in the list.
        return !solvable_cells.empty();
    }

    bool resolve_hidden_single( std::stringstream& annotations )
    {
        // Search the grid for cells that can only have one possible value.
        // For each cell, check the intersection of it's possible values
        // against the possible values of the other cells in the intersecting
        // row, column, and box.  If there is exactly one value in the cell
        // that none of the other cells in an intersecting block can have, then
        // this cell is a hidden single. Remove all other possible values from
        // the cell.
        bool made_change = false;

        std::stringstream local_annotation;

        block_array_t& box_array = block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ];
        for( size_t box_index = 0; box_index < grid_size; ++box_index )
        {
            std::shared_ptr<block>& box_ptr = box_array[ box_index ];
            cell_array_t::iterator cell_it = box_ptr->get_cells().begin();

            while( cell_it != box_ptr->get_cells().end() )
            {
                // Skip cells that are already solved.
                if( ( *cell_it )->get_possible_values().empty() )
                {
                    ++cell_it;
                    continue;
                }

                // There should be no cells with exactly one possible value at
                // this point in the solution cycle.
                ASSERT( !( *cell_it )->has_exactly_one_value() );

                // Current cell has more than one possible value, see if it has
                // a single possible value that no other intersecting block
                // cell has.  Cycle through all of the block types for this
                // cell checking the intersection of possible values.
                std::list<cell_value_t> cell_possibles = ( *cell_it )->get_possible_values();
                for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
                {
                    // Get the correct block.
                    std::vector<size_t> intersecting_block_index;
                    intersecting_block_index.resize( BLOCK_TYPE::BLOCK_TYPE_MAX );

                    intersecting_block_index[ BLOCK_TYPE::BLOCK_TYPE_BOX ] = ( *cell_it )->get_box();
                    intersecting_block_index[ BLOCK_TYPE::BLOCK_TYPE_ROW ] = ( *cell_it )->get_row();
                    intersecting_block_index[ BLOCK_TYPE::BLOCK_TYPE_COL ] = ( *cell_it )->get_column();

                    std::shared_ptr<block>& intersecting_block = block_arrays[ block_type ][ intersecting_block_index[ block_type ] ];

                    // Check if this cell has a unique value in the block.
                    cell_value_t unique_value = intersecting_block->check_block_cell_for_unique_possible( ( *cell_it ).get() );
                    if( 0 != unique_value )
                    {
                        // This cell has exactly one value that none of the
                        // other cells in this block can have. That value must
                        // be the solution for this cell.
                        if( !local_annotation.str().empty() )
                        {
                            local_annotation << ", ";
                        }

                        local_annotation <<
                            ( *cell_it )->get_row_printable() <<
                            ( *cell_it )->get_column_printable() <<
                            " (" << static_cast<char>( unique_value + '0' ) << "," <<
                            " " << block_type_to_text( static_cast<BLOCK_TYPE>( block_type ) ) << ")";

                        // Clear out this cell's other possible values.
                        ( *cell_it )->set_single_possible( unique_value );
                        made_change = true;

                        // Move to the next cell in the block.
                        break;
                    }
                }

                ++cell_it;
            }
        }

        if( made_change )
        {
            annotations << "Hidden single";
            if( 11 < local_annotation.str().length() )
            {
                annotations << "s";
            }
            annotations << " @ ";
            annotations << local_annotation.str();
            annotations << "\n\t";
        }

        return made_change;
    }

    bool resolve_pointing_pairs( std::stringstream& annotations )
    {
        // Two cells that live at the intersection of a box and row or column
        // and have a set of shared possible values that are not possible in
        // the rest of the box. Since the value must be in one of the two cells
        // in the box, it can be removed from the intersecting row or column.
        bool made_change = false;

        std::stringstream local_annotation;

        for( size_t box_index = 0; box_index < grid_size; ++box_index )
        {
            std::shared_ptr<block>& box_ptr = block_arrays[ BLOCK_TYPE_BOX ][ box_index ];

            for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
            {
                if( BLOCK_TYPE_BOX == block_type )
                {
                    continue;
                }

                for( size_t block_offset = 0; block_offset < box_size; ++block_offset )
                {
                    // Get the intersection of the possible values for the
                    // three cells in this box's intersecting row or column.
                    cell_array_t box_cells;
                    get_box_cell_intersect( static_cast<BLOCK_TYPE>( block_type ), box_index, block_offset, box_cells );

                    std::list<cell_value_t> locked_candidates;
                    for( size_t cell_index = 1; cell_index < box_size; ++cell_index )
                    {
                        std::set_intersection(
                            box_cells[ 0 ]->get_possible_values().begin(),
                            box_cells[ 0 ]->get_possible_values().end(),
                            box_cells[ cell_index ]->get_possible_values().begin(),
                            box_cells[ cell_index ]->get_possible_values().end(),
                            std::back_inserter( locked_candidates )
                        );
                    }

                    if( locked_candidates.empty() )
                    {
                        // The cells in this row or column of the box have no
                        // common possible values. Move to the next row.
                        continue;
                    }
                    locked_candidates.sort();
                    locked_candidates.unique();

                    // Cells have at least one common value.  Check the cells
                    // in the intersecting row/column to see if any of them
                    // can have any of the common values. If so, then the value
                    // is not a "locked candidate" value.
                    std::shared_ptr<block>& intersecting_block =
                        get_intersecting_block( static_cast<BLOCK_TYPE>( block_type ), box_index, block_offset );

                    for( std::shared_ptr<cell>& check_cell : intersecting_block->get_cells() )
                    {
                        // Skip checking of the box's row and column cells.
                        if( std::find( box_cells.begin(), box_cells.end(), check_cell ) != box_cells.end() )
                        {
                            continue;
                        }

                        // Check cell is outside of the box, see if it has any
                        // possible values in common with potential locked
                        // candidate values.
                        std::list<cell_value_t> invalid_candidates;
                        std::set_intersection(
                            locked_candidates.begin(),
                            locked_candidates.end(),
                            check_cell->get_possible_values().begin(),
                            check_cell->get_possible_values().end(),
                            std::back_inserter( invalid_candidates )
                        );

                        // Remove common values from the locked candidate list.
                        for( cell_value_t invalid : invalid_candidates )
                        {
                            locked_candidates.remove( invalid );
                        }

                        // Short circuit if there are no remaining candidates.
                        if( locked_candidates.empty() )
                        {
                            break;
                        }
                    }

                    // There is at least one valid locked candidate. Remove the
                    // locked candidate values from the remaining cells in the
                    // box.
                    if( !locked_candidates.empty() )
                    {
                        for( std::shared_ptr<cell>& box_cell : box_ptr->get_cells() )
                        {
                            // Skip removal of the candidate values from the
                            // intersecting cells in the box.
                            if( std::find( box_cells.begin(), box_cells.end(), box_cell ) != box_cells.end() )
                            {
                                continue;
                            }

                            for( cell_value_t locked_candidate : locked_candidates )
                            {
                                if( box_cell->can_have_value( locked_candidate ) )
                                {
                                    box_cell->clear_possible( locked_candidate );

                                    if( local_annotation.str().size() )
                                    {
                                        local_annotation << ", ";
                                    }

                                    local_annotation <<
                                        box_cell->get_row_printable() <<
                                        box_cell->get_column_printable() <<
                                        " (" << static_cast<char>( locked_candidate + '0' ) << "," <<
                                        " " << block_type_to_text( static_cast<BLOCK_TYPE>( block_type ) ) << ")";

                                    made_change = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if( made_change )
        {
            annotations << "Pointing pair @ ";
            annotations << local_annotation.str();
            annotations << "\n\t";
        }

        return made_change;
    }

    bool resolve_naked_pairs( std::stringstream& annotations )
    {
        // Two cells in a row, column, or box each contain the same two
        // possible values, and only those two possibles. Those values
        // can be eliminated from the other cells in the block.
        bool strategy_successful = false;

        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            for( size_t block_index = 0; block_index < grid_size; ++block_index )
            {
                // Look for cells with exactly two possible values.
                std::shared_ptr<block>& block_ptr = block_arrays[ block_type ][ block_index ];

                std::vector<std::pair<cell*, cell*>> pairs;
                if( !block_ptr->get_naked_hidden_pairs( pairs, true ) )
                {
                    continue;
                }

                // At this point the cells in the match lists all have the
                // exact same set of possible values.  Clear these possible
                // values from the other cells in all of the intersecting
                // blocks.
                for( std::pair<cell*, cell*> pair : pairs )
                {
                    std::stringstream local_annotation;
                    strategy_successful = clear_block_matching_values( pair, local_annotation );

                    if( strategy_successful )
                    {
                        annotations << "Naked pair @ " <<
                            pair.first->get_row_printable() <<
                            pair.first->get_column_printable() << "," <<
                            pair.second->get_row_printable() <<
                            pair.second->get_column_printable() << ":";
                        annotations << local_annotation.str();
                        annotations << "\n\t";
                    }
                }
            }
        }

        return strategy_successful;
    }

    bool resolve_hidden_pairs( std::stringstream& annotations )
    {
        // Two cells in a row, column, or box each contain the same two
        // possible values, and none of the other cells in the block
        // contain either of those values. The two matching cells can
        // remove all possibles except the two that are common.
        bool made_change = false;

        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            for( size_t block_index = 0; block_index < grid_size; ++block_index )
            {
                // Look for cells with exactly two possible values.
                std::shared_ptr<block>& block_ptr = block_arrays[ block_type ][ block_index ];

                std::vector<std::pair<cell*, cell*>> pairs;
                if( !block_ptr->get_naked_hidden_pairs( pairs, false ) )
                {
                    continue;
                }

                // At this point each pair of cells in the match list has a
                // common set of possible values.  Clear the impossible values
                // from each of the cells in the pair.
                for( std::pair<cell*, cell*>& pair : pairs )
                {
                    std::stringstream local_annotation;
                    bool strategy_successful = clear_cell_nonmatching_values( pair, local_annotation );

                    if( strategy_successful )
                    {
                        made_change = true;

                        annotations << "Hidden pair @ " <<
                            pair.first->get_row_printable() <<
                            pair.first->get_column_printable() << "," <<
                            pair.second->get_row_printable() <<
                            pair.second->get_column_printable() << ":";
                        annotations << local_annotation.str();
                        annotations << "\n\t";
                    }
                }
            }
        }

        return made_change;
    }

    void get_box_cell_intersect(
        BLOCK_TYPE block_type,
        size_t box_index,
        size_t start_offset,
        cell_array_t& box_cells )
    {
        // Call only valid for grabbing a set of three cells from a box's
        // intersecting row or column.
        ASSERT( BLOCK_TYPE_BOX != block_type );
        ASSERT( 0 <= start_offset && start_offset < box_size );

        size_t block_index;
        if( BLOCK_TYPE_ROW == block_type )
        {
            block_index = ( ( box_index / box_size ) * box_size ) + start_offset;
        }
        else
        {
            block_index = ( ( box_index % box_size ) * box_size ) + start_offset;
        }

        std::shared_ptr<block>& block_ptr = block_arrays[ block_type ][ block_index ];
        cell_array_t::iterator start = block_ptr->get_cells().begin();

        // Calculate the cell offset in the row or column block list based on
        // the box index.
        size_t cell_offset =
            block_arrays[ BLOCK_TYPE_BOX ][ box_index ]->get_extents()[( BLOCK_TYPE_ROW == block_type ) ? 2 : 0];
        start += cell_offset;

        for( size_t cell_index = 0; cell_index < box_size; ++cell_index )
        {
            box_cells.emplace_back( *start );
            ++start;
        }
    }

    std::shared_ptr<block>& get_intersecting_block(
        BLOCK_TYPE block_type,
        size_t box_index,
        size_t start_offset )
    {
        // Calculate which row or column is needed based on the box index and
        // the start offset.
        size_t block_index;
        if( BLOCK_TYPE_ROW == block_type )
        {
            block_index = ( ( box_index / box_size ) * box_size ) + start_offset;
        }
        else
        {
            block_index = ( ( box_index % box_size ) * box_size ) + start_offset;
        }
        return block_arrays[ block_type ][ block_index ];
    }
#if 0
    bool find_matching_possibles(
        cell_array_t::iterator cur_block_cell,
        cell_array_t::iterator end_block_cell,
        std::array<std::tuple<std::shared_ptr<cell>, cell_value_t, cell_value_t>, 2>& matching_cells )
    {
        // Try to find the following:
        // a) two cells in the block that are the only two cells which have two
        //    matching possible values;
        // b) two cells in the block that are the only two cells which have one
        //    matching possible value; or
        // c) one cell in the block that has a unique possible value.
        cell_array_t::iterator next_block_cell = cur_block_cell; next_block_cell++;

        std::map<cell_value_t, bool> uniques;
        std::multimap<cell_value_t, std::shared_ptr<cell>> matches_1;
        std::multimap<std::pair<cell_value_t, cell_value_t>, std::shared_ptr<cell>> matches_2;

        while( next_block_cell != end_block_cell )
        {
            if( ( *next_block_cell )->get_possible_values().empty() )
            {
                ++next_block_cell;
                continue;
            }

            std::list<cell_value_t> intersection;
            std::set_intersection(
                ( *cur_block_cell )->get_possible_values().begin(),
                ( *cur_block_cell )->get_possible_values().end(),
                ( *next_block_cell )->get_possible_values().begin(),
                ( *next_block_cell )->get_possible_values().end(),
                std::back_inserter( intersection )
            );

            if( 2 == intersection.size() )
            {
                // Current and next cell have two matching possible values.
                cell_value_t value_1 = *( intersection.begin() );
                cell_value_t value_2 = *( intersection.rbegin() );

                matches_2.emplace( std::make_pair( std::make_pair( value_1, value_2 ), *next_block_cell ) );
            }
            else if( 1 == intersection.size() )
            {
                // Current and next cell have one matching possible value.
                cell_value_t value = *( intersection.begin() );
                matches_1.emplace( std::make_pair( value, *next_block_cell ) );
            }

            ++next_block_cell;
        }

        bool found_match = false;
        if( 1 == matches_2.size() )
        {
            // Exactly one other cell that has two matching possibles.
            cell_value_t value_1 = matches_2.begin()->first.first;
            cell_value_t value_2 = matches_2.begin()->first.second;

            matching_cells[ 0 ] = std::make_tuple( *cur_block_cell, value_1, value_2 );
            matching_cells[ 1 ] = std::make_tuple( matches_2.begin()->second, value_1, value_2);

            found_match = true;
        }
        else if( 1 == matches_1.size() )
        {
            // Exactly one other cell that has one matching possible.
            cell_value_t value_1 = matches_1.begin()->first;

            matching_cells[ 0 ] = std::make_tuple( *cur_block_cell, value_1, 0 );
            matching_cells[ 1 ] = std::make_tuple( matches_1.begin()->second, value_1, 0);

            found_match = true;
        }
        else if( !uniques.empty() )
        {
            for( std::pair<const cell_value_t, bool>& unique : uniques )
            {
                if( unique.second )
                {
                    // Check there is exactly one unique value in this cell
                    matching_cells[ 0 ] = std::make_tuple( *cur_block_cell, unique.first, 0 );

                    if( found_match )
                    {
                        // More than one value in the uniques list, so no unique value
                        found_match = false;
                        break;
                    }

                    found_match = true;
                }
            }
        }

        // Return true if the data in matching_cells is valid.
        return found_match;
    }
#endif

    void clear_value( cell* block_cell, cell_value_t val )
    {
        // Remove the value from the block's needed list and the cell's
        // possible values list.
        size_t block_index = block_cell->get_row();
        block* block_ptr = block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ][ block_index ].get();
        block_ptr->unset_needed( val );
        for( std::shared_ptr<cell>& clear_cell : block_ptr->get_cells() )
        {
            clear_cell->clear_possible( val );
        }

        block_index = block_cell->get_column();
        block_ptr = block_arrays[ BLOCK_TYPE::BLOCK_TYPE_COL ][ block_index ].get();
        block_ptr->unset_needed( val );
        for( std::shared_ptr<cell>& clear_cell : block_ptr->get_cells() )
        {
            clear_cell->clear_possible( val );
        }

        block_index = block_cell->get_box();
        block_ptr = block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ][ block_index ].get();
        block_ptr->unset_needed( val );
        for( std::shared_ptr<cell>& clear_cell : block_ptr->get_cells() )
        {
            clear_cell->clear_possible( val );
        }
    }

    void pre_set_verify( const cell* solved_cell, cell_value_t value ) const
    {
        if( is_debug_enabled )
        {
            ASSERT( ( 1 <= value ) && ( value <= grid_size ) );

            // Check that this value does not already exist in the cell's
            // intersecting blocks.
            std::array<std::shared_ptr<block>, 3> cell_blocks = {
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ][ solved_cell->get_box() ],
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ][ solved_cell->get_row() ],
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_COL ][ solved_cell->get_column() ]
            };

            for( std::shared_ptr<block>& block_ptr : cell_blocks )
            {
                // Verify the solved value is unique in each intersecting block.
                for( std::shared_ptr<cell>& block_cell : block_ptr->get_cells() )
                {
                    if( block_cell.get() == solved_cell )
                    {
                        continue;
                    }

                    cell_value_t cell_value = block_cell->get_value();
                    if( 0 != cell_value )
                    {
                        // Comparison cell is solved. Verify it's value is not the
                        // same as the value being set for this cell.
                        ASSERT( cell_value != value );
                    }
                }
            }
        }
    }

    void post_set_verify( const cell* solved_cell, cell_value_t value )
    {
        if( is_debug_enabled )
        {
            // Check that this value does not already exist in the cell's
            // intersecting blocks.
            std::array<std::shared_ptr<block>, 3> cell_blocks = {
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_BOX ][ solved_cell->get_box() ],
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_ROW ][ solved_cell->get_row() ],
                block_arrays[ BLOCK_TYPE::BLOCK_TYPE_COL ][ solved_cell->get_column() ]
            };

            for( std::shared_ptr<block>& block_ptr : cell_blocks )
            {
                std::list<cell_value_t> block_needed = block_ptr->get_needed_values();

                // Verify the solved value is no longer in the block needed list.
                ASSERT( std::find( block_needed.begin(), block_needed.end(), value ) == block_needed.end() );
            }
        }
    }

    bool array_matching_pairs(
        cell_array_t& cells_to_compare,
        std::vector<std::pair<std::shared_ptr<cell>, std::shared_ptr<cell>>>& cells_that_match
    )
    {
        // Need to find the matching pairs of possible values. Walk the list of
        // cells to compare and create a list of possible value pairs.
        cell_array_t::iterator compare_it = cells_to_compare.begin();
        while( compare_it != cells_to_compare.end() )
        {
            cell_array_t::iterator next_it = compare_it; ++next_it;
            while( next_it != cells_to_compare.end() )
            {
                std::list<cell_value_t> intersecting_values;
                std::set_intersection(
                    ( *compare_it )->get_possible_values().begin(),
                    ( *compare_it )->get_possible_values().end(),
                    ( *next_it )->get_possible_values().begin(),
                    ( *next_it )->get_possible_values().end(),
                    std::back_inserter( intersecting_values )
                );

                if( 2 == intersecting_values.size() )
                {
                    cell_value_t value1 = intersecting_values.front();
                    cell_value_t value2 = intersecting_values.back();

                    cells_that_match.emplace_back( std::make_pair( *compare_it, *next_it ) );
                }

                ++next_it;
            }

            ++compare_it;
        }

        if( 1 < cells_that_match.size() )
        {
            scrub_intersecting_pairs( cells_that_match );
        }

        return !cells_that_match.empty();
    }

    void scrub_intersecting_pairs(
        std::vector<std::pair<std::shared_ptr<cell>, std::shared_ptr<cell>>>& cells_that_match )
    {
        // A cell may only appear in the list one time. If a cell appears
        // more than once it means the pairs do not qualify for naked or
        // hidden pair operations. Filter out entries with duplicate cells.
        std::list<std::shared_ptr<cell>> unique_cells;

        // Build a list of all of the cells in the intersecting pairs array.
        std::vector<std::pair<std::shared_ptr<cell>, std::shared_ptr<cell>>>::iterator base_it;
        base_it = cells_that_match.begin();
        while( base_it != cells_that_match.end() )
        {
            unique_cells.emplace_back( ( *base_it ).first );
            unique_cells.emplace_back( ( *base_it ).second );

            ++base_it;
        }

        // Remove any cells that are duplicated.
        unique_cells.sort( cell::cell_sort );
        std::list<std::shared_ptr<cell>>::iterator cell_it = unique_cells.begin();
        while( cell_it != unique_cells.end() )
        {
            std::list<std::shared_ptr<cell>>::iterator next_it;
            next_it = cell_it; ++next_it;
            while( next_it != unique_cells.end() )
            {
                if( *cell_it != *next_it )
                {
                    break;
                }

                ++next_it;
            }

            if( 1 != std::distance( cell_it, next_it ) )
            {
                cell_it = unique_cells.erase( cell_it, next_it );
            }
            else
            {
                ++cell_it;
            }
        }

        // Now, go through the list of intersecting pairs one more time.
        // Remove any entries whose cells are not in the unique entry list.
        base_it = cells_that_match.begin();
        while( base_it != cells_that_match.end() )
        {
            if( std::find( unique_cells.begin(), unique_cells.end(), ( *base_it ).first ) == unique_cells.end() ||
                std::find( unique_cells.begin(), unique_cells.end(), ( *base_it ).second ) == unique_cells.end() )
            {
                // One or both of the cells in this intersecting pair are
                // missing from the unique list, so remove this entry from
                // the intersecting pairs list.
                base_it = cells_that_match.erase( base_it );
                continue;
            }

            ++base_it;
        }
    }

    bool clear_block_matching_values(
        std::pair<cell*, cell*>& cell_pair,
        std::stringstream& annotations
    )
    {
        // The possible values for the cell pair must be identical.
        ASSERT( cell_pair.first->get_possible_values() ==
                cell_pair.second->get_possible_values() );

        // Determine which block(s) are shared between the cells. Cell common
        // values should be cleared from all shared blocks.
        std::vector<std::pair<bool, size_t>> block_infos; block_infos.resize( BLOCK_TYPE_MAX );

        block_infos[ BLOCK_TYPE_BOX ] = std::make_pair(
            ( cell_pair.first->get_box() == cell_pair.second->get_box() ),
            cell_pair.first->get_box()
        );
        block_infos[ BLOCK_TYPE_ROW ] = std::make_pair(
            ( cell_pair.first->get_row() == cell_pair.second->get_row() ),
            cell_pair.first->get_row()
        );
        block_infos[ BLOCK_TYPE_COL ] = std::make_pair(
            ( cell_pair.first->get_column() == cell_pair.second->get_column() ),
            cell_pair.first->get_column()
        );

        // Validate that the second cell of the pair is in at least one of the
        // same blocks as the first cell in the pair.
        ASSERT( block_infos[ 0 ].first ||
                block_infos[ 1 ].first ||
                block_infos[ 2 ].first );

        // Only need the block data from the first cell (the second cell of the
        // pair is guaranteed to be in the same block).
        bool made_change = false;
        for( size_t block_type = BLOCK_TYPE::BLOCK_TYPE_MIN; block_type < BLOCK_TYPE::BLOCK_TYPE_MAX; ++block_type )
        {
            if( !block_infos[ block_type ].first )
            {
                // This block is not common between the cells, skip clearing of
                // the shared possible values.
                continue;
            }

            std::shared_ptr<block>&this_block = block_arrays[ block_type ][ block_infos[ block_type ].second ];
            for( cell_value_t clear_value : cell_pair.first->get_possible_values() )
            {
                bool cleared_value = false;
                std::stringstream local_annotations;
                local_annotations << "\n\t\tclearing value " << clear_value << " from cells ";

                for( std::shared_ptr<cell>& clear_cell : this_block->get_cells() )
                {
                    if( ( clear_cell.get() == cell_pair.first ) || ( clear_cell.get() == cell_pair.second ) )
                    {
                        // Current clear cell is pointing at one of cells that
                        // should not be cleared, skip it and move to the next
                        // cell in the block
                        continue;
                    }
                    else if( clear_cell->can_have_value( clear_value ) )
                    {
                        local_annotations <<
                            clear_cell->get_row_printable() <<
                            clear_cell->get_column_printable() << " ";

                        // Clear the matching cell's possible values from this
                        // block cell. Possible values for the cell pair must
                        // be identical, so just use the values from the first
                        // cell in the pair.
                        clear_cell->clear_possible( clear_value );

                        cleared_value = true;
                        made_change = true;
                    }
                }

                if( cleared_value )
                {
                    annotations << local_annotations.str();
                }
            }
        }

        return made_change;
    }

    bool clear_cell_nonmatching_values(
        std::pair<cell*, cell*>& pair,
        std::stringstream& annotations
    )
    {
        // For each of the cells in the list, clear possible values that are
        // not common between the pair of cells.
        // Build the common value list.
        std::vector<cell_value_t> common_values;
        std::set_intersection(
            pair.first->get_possible_values().begin(),
            pair.first->get_possible_values().end(),
            pair.second->get_possible_values().begin(),
            pair.second->get_possible_values().end(),
            std::back_inserter( common_values )
        );

        bool made_change = false;

        std::vector<cell*> cell_array = { pair.first, pair.second };
        for( cell* cur_cell : cell_array )
        {
            // Build the non-common value list for the first cell in the pair
            // and use that list to unset the impossible values.
            std::vector<cell_value_t> clear_values;
            std::set_difference(
                cur_cell->get_possible_values().begin(),
                cur_cell->get_possible_values().end(),
                common_values.begin(),
                common_values.end(),
                std::back_inserter( clear_values )
            );

            if( !clear_values.empty() )
            {
                std::vector<cell_value_t> changed;

                for( cell_value_t clear_value : clear_values )
                {
                    if( cur_cell->can_have_value( clear_value ) )
                    {
                        cur_cell->clear_possible( clear_value );
                        changed.emplace_back( clear_value );
                        made_change = true;
                    }
                }

                if( made_change )
                {
                    annotations << "\n\t\tclearing value";
                    if( 1 < changed.size() )
                    {
                        annotations << "s";
                    }
                    annotations << " ";

                    for( cell_value_t clear_value : changed )
                    {
                        annotations << clear_value << " ";
                    }

                    annotations << "from cell " <<
                        cur_cell->get_row_printable() <<
                        cur_cell->get_column_printable();
                }
            }
        }


        return made_change;
    }

    const char* block_type_to_text( BLOCK_TYPE block_type )
    {
        static std::array<std::string, BLOCK_TYPE_MAX> block_type_to_name;
        block_type_to_name[ BLOCK_TYPE_BOX ] = "box";
        block_type_to_name[ BLOCK_TYPE_ROW ] = "row";
        block_type_to_name[ BLOCK_TYPE_COL ] = "col";

        return block_type_to_name[ block_type ].c_str();
    }

    void print_possibles_grid_header( size_t row_width, size_t col_width )
    {
        // Print possible values grid two line header.
        std::cout << "Possible values grid:\n";

        for( size_t console_col = 0; console_col < row_width + 1; ++console_col )
        {
            std::cout << " ";
        }
        for( size_t column_index = 0; column_index < grid_size; ++column_index )
        {
            for( size_t cell_column = 0; cell_column < col_width; ++cell_column )
            {
                if( ( col_width / 2 ) == cell_column )
                {
                    std::cout << static_cast<char>( column_index + 1 + '0' );
                }
                else
                {
                    std::cout << " ";
                }
            }
        }
        std::cout << "\n";
    }

    void print_possibles_grid_divider( size_t row_width, size_t col_width )
    {
        for( size_t console_col = 0; console_col < row_width + 1; ++console_col )
        {
            std::cout << " ";
        }

        for( size_t column_index = 0; column_index < grid_size; ++column_index )
        {
            for( size_t cell_column = 0; cell_column < col_width; ++cell_column )
            {
                if( 0 == cell_column )
                {
                    std::cout << '+';
                }
                else
                {
                    std::cout << "-";
                }
            }
        }
        std::cout << "+\n";
    }

    size_t grid_size;
    size_t box_size;

    PROGRAM_ARGS parameters;

    std::array<block_array_t, BLOCK_TYPE::BLOCK_TYPE_MAX> block_arrays;
};