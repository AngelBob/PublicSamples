// Day17.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

enum REGISTERS
{
    REG_A = 0,
    REG_B,
    REG_C
};

static bool read_input(
    std::array<uint32_t, 3>& registers,
    std::vector<uint8_t>& op_codes
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Registers.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    size_t reg_idx = 0;
    while( std::getline( file, line, ',' ) )
    {
        registers[ reg_idx ] = stol( line );
        reg_idx++;
    }

    file.close();
    file.open( ".\\Asm.txt" );
    while( std::getline( file, line, ',' ) )
    {
        op_codes.emplace_back( static_cast<uint8_t>( stoul( line ) ) );
    }

    // Step 3: return success or failure.
    return true;
}

static uint32_t get_combo(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    uint32_t result = 0;
    switch( combo )
    {
    case 0:
    case 1:
    case 2:
    case 3:
        result = combo;
        break;
    case 4:
    case 5:
    case 6:
        result = registers[ ( combo - 4 ) ];
        break;
    case 7:
    default:
        __debugbreak();  // Should not happen
    }

    return result;
}

static double do_div(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    double numerator = registers[ REG_A ];
    double denominator = get_combo( combo, registers );

    denominator = std::pow( 2, denominator );

    return numerator / denominator;
}

static void do_adv(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    registers[ REG_A ] = static_cast<uint32_t>( do_div( combo, registers ) );
}

static void do_bdv(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    registers[ REG_B ] = static_cast<uint32_t>( do_div( combo, registers ) );
}

static void do_cdv(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    registers[ REG_C ] = static_cast<uint32_t>( do_div( combo, registers ) );
}

static void do_bxl(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    uint32_t a = registers[ REG_B ];
    uint32_t b = combo;

    registers[ REG_B ] = a ^ b;
}

static void do_bxc(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    uint32_t a = registers[ REG_B ];
    uint32_t b = registers[ REG_C ];

    registers[ REG_B ] = a ^ b;
}

static void do_bst(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers
)
{
    uint32_t a = get_combo( combo, registers );

    registers[ REG_B ] = ( a & 0x7 );
}

static void do_out(
    const uint8_t combo,
    std::array<uint32_t, 3>& registers,
    std::string& output
)
{
    uint32_t a = get_combo( combo, registers );

    if( output.length() )
    {
        output += ",";
    }
    output += std::to_string( a & 0x7 );
}

static void execute(
    std::array<uint32_t, 3>& registers,
    const std::vector<uint8_t>& op_codes,
    std::string& output
)
{
    std::vector<uint8_t>::const_iterator op = op_codes.cbegin();
    while( op_codes.cend() != op )
    {
        std::vector<uint8_t>::const_iterator combo = std::next( op );
        if( op_codes.cend() == combo )
        {
            // This shouldn't happen...
            __debugbreak();
        }

        switch( *op )
        {
        case 0:
            // adv - division out to A
            do_adv( *combo, registers );
            break;
        case 1:
            // bxl - bitwise OR w/ operand
            do_bxl( *combo, registers );
            break;
        case 2:
            // bst - mod 8
            do_bst( *combo, registers );
            break;
        case 3:
            // jnz - jump if not zero
            if( registers[ REG_A ] )
            {
                op = std::next( op_codes.cbegin(), ( *combo ) );
                continue;
            }
            break;
        case 4:
            // bxc - bitwise OR on B & C
            do_bxc( *combo, registers );
            break;
        case 5:
            // out - output
            do_out( *combo, registers, output );
            break;
        case 6:
            // bdv - division out to B
            do_bdv( *combo, registers );
            break;
        case 7:
            // cdv - division out to C
            do_cdv( *combo, registers );
            break;
        }

        op = std::next( op, 2 );
    }
}

int main()
{
    std::array<uint32_t, 3> registers;
    std::vector<uint8_t> op_codes;
    if( !read_input( registers, op_codes ) )
    {
        return -1;
    }

    std::string output;
    execute( registers, op_codes, output );

    std::cout << output << std::endl;
}
