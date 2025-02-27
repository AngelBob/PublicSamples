// SudokuSolver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define __STDC_WANT_LIB_EXT1__ 1

#include "debug.h"
#include "sudoku_types.h"

#include "block.hpp"
#include "cell.hpp"
#include "grid.hpp"

static const std::array<const char*, SEARCH_STRATEGY::STRATEGY_END> strategy_test_files = {
    "test0_hidden_single.txt",
    "test1_pointing_pair.txt",
    "test2_naked_pair.txt",
    "test3_hidden_pair.txt",
    "test4_naked_triplet.txt",
    "test5_hidden_triplet.txt",
    "test6_x_wing.txt",
    "test7_xy_wing.txt",
    "test8_swordfish.txt"
};

bool get_strategy( char this_number, uint32_t& strategy )
{
    bool have_value = false;

    if( ( '0' <= this_number ) && ( this_number <= '9' ) )
    {
        strategy *= 10;
        strategy += ( this_number - '0' );

        have_value = true;
    }

    return ( have_value &&
             ( ( SEARCH_STRATEGY::STRATEGY_HIDDEN_SINGLE <= strategy ) &&
               ( strategy < SEARCH_STRATEGY::STRATEGY_END )
             )
           );
}

bool parse_args( std::vector<std::string> parameter_strings, PROGRAM_ARGS& parsed_parameters )
{
    uint32_t param_type = 0;
    uint32_t annotation_bits = 0;

    uint32_t start_strategy = SEARCH_STRATEGY::STRATEGY_HIDDEN_SINGLE;
    bool have_strategy = false;

    char last_parameter = '\0';
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
            // No leading dash, either a file name or a test parameter
            if( 't' == last_parameter )
            {
                for( char this_character : parameter )
                {
                    have_strategy = get_strategy( this_character, start_strategy );
                    if( !have_strategy )
                    {
                        start_strategy = 0;
                    }
                }
            }
            else
            {
                parsed_parameters.input_file = parameter;
            }
            break;

        case 1:
            {
                // Single leading dash, so a list of short parameters
                for( char this_character : parameter )
                {
                    switch( this_character )
                    {
                    case 'v':
                        if( 'v' == this_character )
                        {
                            if( 'v' == last_parameter )
                            {
                                annotation_bits <<= 1;
                                annotation_bits |= 1;
                            }
                            else
                            {
                                annotation_bits = 1;
                            }
                            last_parameter = 'v';
                        }
                        break;

                    case 't':
                        // Test run; which test?
                        last_parameter = 't';
                        break;
                    
                    case '?':
                    case 'h':
                        // Print the help text and exit
                        return false;

                    default:
                        if( 't' == last_parameter )
                        {
                            have_strategy = get_strategy( this_character, start_strategy );
                            if( !have_strategy )
                            {
                                start_strategy = 0;
                            }
                        }
                        else
                        {
                            last_parameter = '\0';
                        }
                        break;
                    }
                }
            }
            break;

        case 2:
            // Double leading dash, so a long parameter
            if( "verbose" == parameter )
            {
                annotation_bits = 1;
            }
            else if( "noisy" == parameter )
            {
                annotation_bits = 3;
            }
            else if( "test" == parameter )
            {
                last_parameter = 't';
            }
            else if( "help" == parameter )
            {
                return false;
            }
            break;
        }

        param_type = 0;
    }

    // Parsed the params, now translate them into the parameters structure.
    if( have_strategy )
    {
        parsed_parameters.input_file = strategy_test_files[ start_strategy ];
        parsed_parameters.test_strategy = start_strategy;
        parsed_parameters.is_test = true;

        // Testing turns on verbose output for validation
        annotation_bits = 3;
    }

    uint32_t bit = 0;
    while( annotation_bits )
    {
        parsed_parameters.annotations.set( bit++ );
        annotation_bits >>= 1;
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

static bool solve_grid( grid& board, size_t known_value_count, bool is_test )
{
    // Move through each of the cells looking for
    // a cell that can take exactly one value.
    const size_t total_value_count = board.get_grid_size() * board.get_grid_size();

    size_t round = 0;
    bool found_value = true;

    do
    {
        board.print_grid( round++ );
        board.dump_possibles();

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

        board.validate_cells();
        if( !found_value )
        {
            board.dump_possibles();
        }
    } while( ( total_value_count != known_value_count ) && found_value && !is_test );

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
    PROGRAM_ARGS parsed_parameters{};
    if( !parse_args( program_args, parsed_parameters ) )
    {
        print_help();
        return 1;
    }

    board.set_grid_parameters( parsed_parameters );

    fill_grid( board, known_value_count, parsed_parameters.input_file );

    bool solved = solve_grid( board, known_value_count, parsed_parameters.is_test );
    if( solved )
    {
        std::cout << "\nFound a solution:" << std::endl;
    }
    else
    {
        std::cout << "\nDid not find a solution:" << std::endl;
    }

    board.print_grid( static_cast<size_t>( -1 ) );

    return ( solved ) ? 0 : -2;
}
