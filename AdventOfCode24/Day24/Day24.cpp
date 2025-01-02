// Day24.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <tuple>

static bool read_input(
    std::map<std::string, bool>& registers,
    std::map<std::string, std::tuple<std::string, std::string, std::string>>& operations
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Logic.txt");

    // Step 2: read each line and insert operations into the map.
    bool have_registers = false;
    std::string line;
    while( std::getline( file, line ) )
    {
        if (0 == line.length())
        {
            have_registers = true;
            continue;
        }

        if( !have_registers )
        {
            std::string reg( line.substr( 0, line.find( ':' ) ) );
            bool val = std::stoi( line.substr( line.find( ':' ) + 2, line.length() - 1 ) ) ? true : false;

            registers.insert( std::make_pair( reg, val ) );
        }
        else
        {
            std::string first_input( line.substr( 0, 3 ) );
            std::string operation( line.substr( 4, line.find( ' ', 4 ) - 4 ) );
            std::string second_input( line.substr( line.find( ' ', 4 ) + 1, 3 ) );

            std::string output( line.substr( line.length() - 3, line.length() - 1 ) );

            if( 0 != operations.count( output ) )
            {
                // Assert any given output is only used once.
                __debugbreak();
            }
            operations.insert( std::make_pair( output, std::make_tuple( first_input, second_input, operation ) ) );
        }
    }

    // Step 3: return success or failure.
    return( 0 != registers.size() && 0 != operations.size() );
}

static bool resolve_operation(
    const bool operand1,
    const bool operand2,
    const std::string& op
)
{
    if( op == "AND" )
    {
        return ( operand1 && operand2 );
    }
    else if( op == "XOR" )
    {
        return ( operand1 && !operand2 ) || ( !operand1 && operand2 );
    }
    else if( op == "OR" )
    {
        return ( operand1 || operand2 );
    }

    // Unknown operation
    __debugbreak();
    return false;
}

static void resolve_dependencies(
    std::map<std::string, bool>& registers,
    std::map<std::string, std::tuple<std::string, std::string, std::string>>& operations
)
{
    while( !operations.empty() )
    {
        auto op_it( operations.begin() );
        while( operations.end() != op_it )
        {
            if( registers.contains( std::get<0>( op_it->second ) ) &&
                registers.contains( std::get<1>( op_it->second ) )
              )
            {
                bool result = resolve_operation(
                    registers[ std::get<0>( op_it->second ) ],
                    registers[ std::get<1>( op_it->second ) ],
                    std::get<2>( op_it->second ) );

                registers.insert( std::make_pair( op_it->first, result ) );
                op_it = operations.erase( op_it );
            }
            else
            {
                ++op_it;
            }
        }
    }
}

int main()
{
    std::map<std::string, bool> registers;
    std::map<std::string, std::tuple<std::string, std::string, std::string>> operations;
    if( !read_input( registers, operations ) )
    {
        return -1;
    }

    resolve_dependencies( registers, operations );

    uint64_t output = 0;
    uint32_t shift = 0;
    while( 1 )
    {
        std::stringstream reg_num;
        reg_num << "z" << std::setfill('0') << std::setw(2) << shift;
        if( !registers.contains( reg_num.str() ) )
        {
            break;
        }

        if( registers[ reg_num.str() ] )
        {
            output |= static_cast<uint64_t>( 1 ) << shift;
        }

        ++shift;
    }

    std::cout << "The decimal output is " << std::to_string( output ) << "\n";
}
