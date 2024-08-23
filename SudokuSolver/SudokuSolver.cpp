// SudokuSolver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define __STDC_WANT_LIB_EXT1__ 1

#include <algorithm>
#include <conio.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "debug.h"
#include "sudoku_types.h"

#include "block.hpp"
#include "cell.hpp"
#include "grid.hpp"

void print_grid( const size_t round, const grid& grid );

static bool parse_args( std::vector<std::string> parameter_strings, PROGRAM_ARGS& parsed_parameters )
{
    size_t param_type = 0;
    for( std::string& parameter : parameter_strings )
    {
        while( '-' == parameter[ 0 ] )
        {
            parameter.erase( 0, 1 );
            ++param_type;
        }

        switch( param_type )
        {
        case 0:
            // No leading dash, so probably a file name
            parsed_parameters.input_file = parameter;
            break;

        case 1:
            {
                // Single leading dash, so a list of short parameters
                char last_parameter = '\0';
                for( char this_parameter : parameter )
                {
                    switch( this_parameter )
                    {
                    case 'v':
                        if( 'v' == this_parameter )
                        {
                            if( 'v' == last_parameter )
                            {
                                parsed_parameters.annotations.set( ANNOTATION_BITS::ANNOTATIONS_EXTENDED );
                            }
                            else
                            {
                                parsed_parameters.annotations.set( ANNOTATION_BITS::ANNOTATIONS_BASIC );
                            }

                            last_parameter = 'v';
                        }
                        break;

                    case '?':
                    case 'h':
                        // Print the help text and exit
                        return false;

                    default:
                        last_parameter = '\0';
                        break;
                    }
                }
            }
            break;

        case 2:
            // Double leading dash, so a long parameter
            if( "verbose" == parameter )
            {
                parsed_parameters.annotations.set( ANNOTATION_BITS::ANNOTATIONS_BASIC );
            }
            else if( "noisy" == parameter )
            {
                parsed_parameters.annotations.set( ANNOTATION_BITS::ANNOTATIONS_BASIC );
                parsed_parameters.annotations.set( ANNOTATION_BITS::ANNOTATIONS_EXTENDED );
            }
            else if( "help" == parameter )
            {
                return false;
            }
            break;
        }

        param_type = 0;
    }

    return true;
}

static std::vector<cell_value_t> read_row( std::istream& cin, size_t& max_grid )
{
    // Read the streaming input.
    std::vector<cell_value_t> values;

    std::string input;
    bool is_comment = false;

    do 
    {
        std::getline( cin, input );
        is_comment = ( '#' == input[ 0 ] );
    } while ( is_comment );

    cell_value_t value = 0;
    for( char ch : input )
    {
        switch( ch )
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value *= 10;
            value += ch - '0';
            break;

        case ',':
        case '|':
            values.push_back( value );

            // Reset for next input.
            value = 0;
            break;

        default:
            throw( "Invalid value in input grid." );
        }
    }
    values.push_back( value );

    max_grid = std::max( max_grid, values.size() );
    while( values.size() < max_grid )
    {
        values.push_back( 0 );
    }

    return values;
}

static void initialize_grid_data( grid& board )
{
    // Initialize the possible value list for all cells.
    board.initialize_grid();

    // Use the initial value data to update the possible values lists.
    block_array_t rows = board.get_rows();
    for( std::shared_ptr<block>& row : rows )
    {
        cell_array_t& cells = row.get()->get_cells();
        for( std::shared_ptr<cell>& cell : cells )
        {
            cell_value_t value = cell.get()->get_value();
            if( 0 != value )
            {
                board.set_value( cell.get(), value );
            }
        }
    }
}

static void fill_grid( grid& board, size_t& known_value_count, const std::string& file )
{
    bool have_input_file = true;

    std::ifstream ifile;
    ifile.open( file );

    if( !ifile.is_open() )
    {
        // Take user input for initial layout
        std::cout << "Type in the initial layout. 4 x 4 is minimum size grid.\n";
        std::cout << "Enter by row, separating each cell value with a comma (,) or pipe (|).\n";
        std::cout << "The first row must be filled out to the full grid width.\n";
        std::cout << "Use zero (0) as a place-holder for unknown cells.\n";
        std::cout << "It is not necessary to add zero place-holder values at the end of rows except in the first row.\n" << std::endl;

        have_input_file = false;
    }

    size_t max_grid = board.get_grid_size();
    size_t row = 1;

    known_value_count = 0;
    while( row <= max_grid )
    {
        if( !have_input_file )
        {
            std::cout << row << ": ";
        }

        std::vector<cell_value_t> row_values = read_row( have_input_file ? ifile : std::cin, max_grid );
        board.set_grid_size( max_grid );
        if( row_values.size() > max_grid )
        {
            // First row _must_ be the width of the grid.
            throw( "Invalid input - first row too short, or too long row." );
        }

        size_t box_size = board.get_box_size();
        size_t row_idx = row - 1;
        for( size_t col = 0; col < max_grid; ++col )
        {
            size_t box = ( ( row_idx / box_size ) * box_size ) + ( col / box_size );

            if( ( row_values.size() > col ) && ( 0 != row_values[ col ] ) )
            {
                std::shared_ptr<cell> new_cell = std::make_shared<cell>( static_cast<char>( row_idx + 'A' ), col, box, row_values[col] );
                board.add_cell( new_cell );
                ++known_value_count;
            }
            else
            {
                std::shared_ptr<cell> new_cell = std::make_shared<cell>( static_cast<char>( row_idx + 'A' ), col, box);
                board.add_cell( new_cell );
            }
        }

        ++row;
    }

    initialize_grid_data( board );
}

static size_t print_row_leader( const size_t grid_size, const size_t row = static_cast<size_t>( -1 ) )
{
    size_t row_width = 0;
    size_t row_divisor = grid_size / 10;
    do
    {
        ++row_width;
        row_divisor /= 10;
    } while( row_divisor );

    char row_name = ' ';
    if( static_cast<size_t>( -1 ) != row )
    {
        row_name = static_cast<char>( row + 'A' );
    }
    std::cout << std::setw( row_width ) << row_name;

    // Add one space between row numbers and grid
    std::cout << " ";

    return row_width + 1; // Add an extra space between the numbers
}

static void print_col_separator( const size_t col, const size_t box_size, const bool is_separator_row )
{
    std::string spacer( " " );
    std::string separator( "|" );
    if( is_separator_row )
    {
        spacer = "-";
        separator = "+";
    }

    if( 0 == ( col % box_size ) )
    {
        if( 0 != col )
        {
            std::cout << spacer;
        }
        std::cout << separator;
    }
}

static void print_table_header( const size_t round, const size_t box_size )
{
    size_t grid_size = box_size * box_size;

    // Print a blank line first
    std::cout << std::endl;

    // Print the round
    if( 0 == round )
    {
        std::cout << "Starting grid:\n";
    }
    else if( static_cast<size_t>( -1 ) == round )
    {
        std::cout << "Solved grid:\n";
    }
    else
    {
        std::cout << "Solver round " << round << "\n";
    }

    // Offset for row number width
    size_t row_width = print_row_leader( grid_size );

    // Write the column number and skip the separator if necessary
    for( size_t col = 0; col < grid_size; ++col )
    {
        if( 0 == ( col % box_size ) )
        {
            if( 0 != col )
            {
                std::cout << " ";
            }
            std::cout << " ";
        }

        std::cout << std::setw( row_width ) << ( col + 1 );
    }

    // Move to the next line
    std::cout << "\n";
}

static void print_row_separator( const size_t box_size )
{
    size_t grid_size = box_size * box_size;

    size_t row_width = print_row_leader( grid_size );

    for( size_t col = 0; col < grid_size; ++col )
    {
        print_col_separator( col, box_size, true );

        std::cout << std::string( row_width, '-' );
    }
    std::cout << "-+\n";
}

static void print_value( const cell_value_t value, const size_t row_width )
{
    if( 0 != value )
    {
        std::cout << std::setw( row_width ) << static_cast<uint16_t>( value );
    }
    else
    {
        ASSERT( row_width >= 2 );
        std::cout << " ";
        std::cout << std::string( row_width - 1 , '_' );
    }
}

static void print_grid( const size_t round, const grid& board )
{
    size_t grid_size = board.get_grid_size();
    size_t box_size = board.get_box_size();

    print_table_header( round, box_size );

    const block_array_t& rows = board.get_rows();
    size_t row_idx = 0;
    for( const std::shared_ptr<block>& row : rows )
    {
        if( 0 == ( row_idx % box_size ) )
        {
            print_row_separator( box_size );
        }

        size_t row_width = print_row_leader( grid_size, row_idx );

        cell_array_t cells = row.get()->get_cells();
        size_t col_idx = 0;
        for( std::shared_ptr<cell>& cell : cells )
        {
            print_col_separator( col_idx, box_size, false );
            print_value( cell.get()->get_value(), row_width );
            ++col_idx;
        }

        std::cout << " |\n";
        ++row_idx;
    }

    print_row_separator( box_size );

    std::cout << std::endl;
}

static bool solve_grid( grid& board, size_t known_value_count )
{
    // Move through each of the cells looking for
    // a cell that can take exactly one value.
    const size_t total_value_count = board.get_grid_size() * board.get_grid_size();

    size_t round = 1;
    bool found_value = true;

    do
    {
#ifdef _DEBUG
        board.dump_possibles();
#endif

        std::vector<std::pair<cell*, cell_value_t>> solved_cells;
        board.update_possibles( solved_cells );

        if( 0 != solved_cells.size() )
        {
            for( std::pair<cell*, cell_value_t> solution : solved_cells )
            {
                board.set_value( solution.first, solution.second );
            }

            found_value = true;
        }
        else
        {
            found_value = false;
        }

        known_value_count += solved_cells.size();

#ifdef _DEBUG
        print_grid( round++, board );

        board.validate_cells();
        if( !found_value )
        {
            board.dump_possibles();
            ASSERT( found_value );
        }
#endif
    } while( ( total_value_count != known_value_count ) && found_value );

    return ( total_value_count == known_value_count );
}

void print_help( void )
{
    std::cout << "A sudoku puzzle solver.\n";
    std::cout << "usage:  SudokuSolver.exe [options] [file]\n";
    std::cout << "options:\n";
    std::cout << "\t-v --verbose: print operations on possible values.\n";
    std::cout << "\t-vv --noisy: verbose output plus print possible values grid.\n";
    std::cout << "file:\n";
    std::cout << "Optional name of the file containing the initial grid layout.\n";
    std::cout << "If no file name is provided, solver will ask for manual entry of the starting layout.\n";
    std::cout << "4 x 4 is minimum size grid.\n";
    std::cout << "Data laid out by row, separating each cell value with a comma (,) or pipe (|).\n";
    std::cout << "The first row must be filled out to the full grid width.\n";
    std::cout << "Use zero (0) as a place-holder for unknown cells.\n";
    std::cout << "It is not necessary to add zero place-holder values at the end of rows except in the first row.\n";
    std::cout << "Lines beginning with hash (#) are comments." << std::endl;
}

int main( int argc, char* argv[] )
{
    // Cell grid starts empty; user input defines the size of the grid.
    class grid board;
    size_t known_value_count = 0;

    // First parameter string is the executable name, just burn that one.
    std::vector<std::string> program_args( argv + 1, argv + argc );
    PROGRAM_ARGS parsed_parameters;
    if( !parse_args( program_args, parsed_parameters ) )
    {
        print_help();
        return 1;
    }

    board.set_grid_parameters( parsed_parameters );

    fill_grid( board, known_value_count, parsed_parameters.input_file );

    print_grid( 0, board );

    bool solved = solve_grid( board, known_value_count );
    if( solved )
    {
        std::cout << "\nFound a solution:" << std::endl;
    }
    else
    {
        std::cout << "\nDid not find a solution:" << std::endl;
    }

    print_grid( static_cast<size_t>( -1 ), board );

    return ( solved ) ? 0 : -2;
}
