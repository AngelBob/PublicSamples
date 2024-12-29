// Day21.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static const std::map<std::pair<char, char>, std::string> number_pad{{
    {{'0','0'}, "A"},
    {{'0','1'}, "^<A"},
    {{'0','2'}, "^A"},
    {{'0','3'}, "^>A"},
    {{'0','4'}, "^^<A"},
    {{'0','5'}, "^^A"},
    {{'0','6'}, "^^>A"},
    {{'0','7'}, "^^^<A"},
    {{'0','8'}, "^^^A"},
    {{'0','9'}, "^^^>A"},
    {{'0','A'}, ">A"},

    {{'1','0'}, ">vA"},
    {{'1','1'}, "A"},
    {{'1','2'}, ">A"},
    {{'1','3'}, ">>A"},
    {{'1','4'}, "^A"},
    {{'1','5'}, "^>A"},
    {{'1','6'}, "^>>A"},
    {{'1','7'}, "^^A"},
    {{'1','8'}, "^^>A"},
    {{'1','9'}, "^^>>A"},
    {{'1','A'}, ">>vA"},

    {{'2','0'}, "vA"},
    {{'2','1'}, "<A"},
    {{'2','2'}, "A"},
    {{'2','3'}, ">A"},
    {{'2','4'}, "<^A"},
    {{'2','5'}, "^A"},
    {{'2','6'}, "^>A"},
    {{'2','7'}, "<^^A"},
    {{'2','8'}, "^^A"},
    {{'2','9'}, "^^>A"},
    {{'2','A'}, "v>A"},

    {{'3','0'}, "<vA"},
    {{'3','1'}, "<<A"},
    {{'3','2'}, "<A"},
    {{'3','3'}, "A"},
    {{'3','4'}, "<<^A"},
    {{'3','5'}, "<^A"},
    {{'3','6'}, "^A"},
    {{'3','7'}, "<<^^A"},
    {{'3','8'}, "<^^A"},
    {{'3','9'}, "^^A"},
    {{'3','A'}, "vA"},

    {{'4','0'}, ">vvA"},
    {{'4','1'}, "vA"},
    {{'4','2'}, "v>A"},
    {{'4','3'}, "v>>A"},
    {{'4','4'}, "A"},
    {{'4','5'}, ">A"},
    {{'4','6'}, ">>A"},
    {{'4','7'}, "^A"},
    {{'4','8'}, "^>A"},
    {{'4','9'}, "^>>A"},
    {{'4','A'}, ">>vvA"},

    {{'5','0'}, "vvA"},
    {{'5','1'}, "<vA"},
    {{'5','2'}, "vA"},
    {{'5','3'}, "v>A"},
    {{'5','4'}, "<A"},
    {{'5','5'}, "A"},
    {{'5','6'}, ">A"},
    {{'5','7'}, "<^A"},
    {{'5','8'}, "^A"},
    {{'5','9'}, "^>A"},
    {{'5','A'}, "vv>A"},

    {{'6','0'}, "<vvA"},
    {{'6','1'}, "<<vA"},
    {{'6','2'}, "<vA"},
    {{'6','3'}, "vA"},
    {{'6','4'}, "<<A"},
    {{'6','5'}, "<A"},
    {{'6','6'}, "A"},
    {{'6','7'}, "<<^A"},
    {{'6','8'}, "<^A"},
    {{'6','9'}, "^A"},
    {{'6','A'}, "vvA"},

    {{'7','0'}, ">vvvA"},
    {{'7','1'}, "vvA"},
    {{'7','2'}, "vv>A"},
    {{'7','3'}, "vv>>A"},
    {{'7','4'}, "vA"},
    {{'7','5'}, "v>A"},
    {{'7','6'}, "v>>A"},
    {{'7','7'}, "A"},
    {{'7','8'}, ">A"},
    {{'7','9'}, ">>A"},
    {{'7','A'}, ">>vvvA"},

    {{'8','0'}, "vvvA"},
    {{'8','1'}, "<vvA"},
    {{'8','2'}, "vvA"},
    {{'8','3'}, "vv>A"},
    {{'8','4'}, "<vA"},
    {{'8','5'}, "vA"},
    {{'8','6'}, "v>A"},
    {{'8','7'}, "<A"},
    {{'8','8'}, "A"},
    {{'8','9'}, ">A"},
    {{'8','A'}, "vvv>A"},

    {{'9','0'}, "<vvvA"},
    {{'9','1'}, "<<vvA"},
    {{'9','2'}, "<vvA"},
    {{'9','3'}, "vvA"},
    {{'9','4'}, "<<vA"},
    {{'9','5'}, "<vA"},
    {{'9','6'}, "vA"},
    {{'9','7'}, "<<A"},
    {{'9','8'}, "<A"},
    {{'9','9'}, "A"},
    {{'9','A'}, "vvvA"},

    {{'A','0'}, "<A"},
    {{'A','1'}, "^<<A"},
    {{'A','2'}, "<^A"},
    {{'A','3'}, "^A"},
    {{'A','4'}, "^^<<A"},
    {{'A','5'}, "<^^A"},
    {{'A','6'}, "^^A"},
    {{'A','7'}, "^^^<<A"},
    {{'A','8'}, "<^^^A"},
    {{'A','9'}, "^^^A"},
    {{'A','A'}, "A"}
}};

static const std::map<std::pair<char, char>, std::string> direction_pad{{
    {{'A','A'}, "A"},
    {{'A','^'}, "<A"},
    {{'A','>'}, "vA"},
    {{'A','v'}, "<vA"},
    {{'A','<'}, "v<<A"},

    {{'^','A'}, ">A"},
    {{'^','^'}, "A"},
    {{'^','>'}, "v>A"},
    {{'^','v'}, "vA"},
    {{'^','<'}, "v<A"},

    {{'>','A'}, "^A"},
    {{'>','^'}, "<^A"},
    {{'>','>'}, "A"},
    {{'>','v'}, "<A"},
    {{'>','<'}, "<<A"},

    {{'v','A'}, "^>A"},
    {{'v','^'}, "^A"},
    {{'v','>'}, ">A"},
    {{'v','v'}, "A"},
    {{'v','<'}, "<A"},

    {{'<','A'}, ">>^A"},
    {{'<','^'}, ">^A"},
    {{'<','>'}, ">>A"},
    {{'<','v'}, ">A"},
    {{'<','<'}, "A"},
}};

static bool read_input(
    std::vector<std::string>& codes
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Codes.txt");

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    while( std::getline( file, line ) )
    {
        codes.emplace_back( line );
    }

    // Step 3: return success or failure.
    return true;
}

static void do_pad(
    const std::vector<std::string>& input,
    const std::map<std::pair<char, char>, std::string>& pad,
    std::vector<std::string>& movements
)
{
    movements.clear();

    for( const std::string target : input )
    {
        // Movement always starts at the 'A' key.
        movements.emplace_back( pad.at( std::make_pair( 'A', target[ 0 ] ) ) );

        for( size_t idx = 0; idx < target.length() - 1; ++idx )
        {
            movements.emplace_back( pad.at( std::make_pair( target[ idx ], target[ idx + 1 ] ) ) );
        }
    }
}

static void output_strings(
    const std::vector<std::vector<std::string>>& strings_v
)
{
    for( const auto& strings : strings_v )
    {
        for (const auto& string : strings)
        {
            std::cout << string;
        }
        std::cout << "\n\t";
    }
}

int main()
{
    std::vector<std::string> codes;
    if( !read_input( codes ) )
    {
        return -1;
    }

    uint64_t complexity = 0;
    for (const auto& code : codes)
    {
        // Figure out the movements required to enter the code.
        std::vector<std::string> code_v{{ code }};
        std::vector<std::string> move_strings;
        do_pad( code_v, number_pad, move_strings );

        // Translate the code movements across the two intervening robots.
        std::vector<std::string> tx_string;
        do_pad( move_strings, direction_pad, tx_string );

        std::vector<std::string> tx_string1;
        do_pad( tx_string, direction_pad, tx_string1 );

        // Output the calculated strings.
        output_strings( {{ code_v, move_strings, tx_string, tx_string1 }} );

        // Calculate the complexity value for this code.
        uint64_t length = 0;
        for( const std::string& command : tx_string1 )
        {
            length += static_cast<uint64_t>( command.length() );
        }

        uint64_t value = ( code[ 0 ] - '0' ) * 100 + ( code[ 1 ] - '0' ) * 10 + ( code[ 2 ] - '0' );
        complexity += ( length * value );
        std::cout << "(" << std::to_string( length ) << " * " << std::to_string( value ) << ")\n";
    }

    std::cout << "The code complexity is " << std::to_string( complexity ) << "\n";
}