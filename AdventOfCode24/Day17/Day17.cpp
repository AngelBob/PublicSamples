// Day17.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

class computer
{
public:
    computer() = delete;

    computer( const std::array<uint64_t, 3> r ) :
        registers( r )
    {
    }

    void run(
        const std::vector<uint8_t>& op_codes
    )
    {
        while( ip < op_codes.size() )
        {
            uint8_t a = op_codes[ ip ];
            uint8_t b = op_codes[ ip + 1 ];
            exec( a, b );
        }
    }

    const std::vector<uint64_t>& get_output( void ) const
    {
        return output;
    }

private:
    enum REGISTERS
    {
        REG_A = 0,
        REG_B,
        REG_C
    };

    void exec(
        const uint8_t a,
        const uint8_t b
    )
    {
        bool inc_ip = true;

        switch( a )
        {
        case 0:
            // adv - division out to A
            adv( b );
            break;
        case 1:
            // bxl - bitwise OR w/ operand
            bxl( b );
            break;
        case 2:
            // bst - mod 8
            bst( b );
            break;
        case 3:
            // jnz - jump if not zero
            if( registers[ REG_A ] )
            {
                ip = b;
                inc_ip = false;
            }
            break;
        case 4:
            // bxc - bitwise OR on B & C
            bxc( b );
            break;
        case 5:
            // out - output
            out( b );
            break;
        case 6:
            // bdv - division out to B
            bdv( b );
            break;
        case 7:
            // cdv - division out to C
            cdv( b );
            break;
        }

        ip += inc_ip ? 2 : 0;
    }

    uint64_t get_combo(
        const uint8_t combo
    )
    {
        uint64_t result = 0;
        if( combo < 4 )
        {
            result = combo;
        }
        else if( combo < 7 )
        {
            result = registers[ combo - 4 ];
        }
        else
        {
            throw std::runtime_error( "Invalid value for combo" );
        }

        return result;
    }

    uint64_t div(
        const uint8_t combo
    )
    {
        double numerator = static_cast<double>( registers[ REG_A ] );
        double denominator = static_cast<double>( get_combo( combo ) );

        denominator = std::pow( 2, denominator );

        return static_cast<uint64_t>( numerator / denominator );
    }

    void adv(
        const uint8_t combo
    )
    {
        registers[ REG_A ] = div( combo );
    }

    void bdv(
        const uint8_t combo
    )
    {
        registers[ REG_B ] = div( combo );
    }

    void cdv(
        const uint8_t combo
    )
    {
        registers[ REG_C ] = div( combo );
    }

    void bxl(
        const uint8_t combo
    )
    {
        uint64_t a = registers[ REG_B ];
        uint64_t b = combo;

        registers[ REG_B ] = a ^ b;
    }

    void bxc(
        const uint8_t combo
    )
    {
        uint64_t a = registers[ REG_B ];
        uint64_t b = registers[ REG_C ];

        registers[ REG_B ] = a ^ b;
    }

    void bst(
        const uint8_t combo
    )
    {
        uint64_t a = get_combo( combo );

        registers[ REG_B] = ( a & 0x7 );
    }

    void out(
        const uint8_t combo
    )
    {
        uint64_t a = get_combo( combo );
        output.emplace_back( a & 0x7 );
    }

    size_t ip{ 0 };
    std::array<uint64_t, 3> registers;
    std::vector<uint64_t> output;
};

static bool read_input(
    std::array<uint64_t, 3>& registers,
    std::vector<uint8_t>& op_codes
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Registers.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    size_t reg_idx = 0;
    while( std::getline( file, line, ',') )
    {
        registers[ reg_idx ] = stoull( line );
        reg_idx++;
    }

    file.close();
    file.open( ".\\Asm.txt" );
    while( std::getline( file, line, ',') )
    {
        op_codes.emplace_back( static_cast<uint8_t>( stoul( line ) ) );
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::array<uint64_t, 3> registers;
    std::vector<uint8_t> op_codes;
    if( !read_input( registers, op_codes ) )
    {
        return -1;
    }

    computer comp( registers );
    comp.run( op_codes );

    for( auto v : comp.get_output() )
    {
        std::cout << std::to_string( v ) << ",";
    }
    std::cout << std::endl;
}
