// Day24.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>

class operation_node
{
public:
    operation_node() = delete;

    operation_node(
        const std::string& out,
        std::shared_ptr<operation_node> in1,
        std::shared_ptr<operation_node> in2,
        const std::string& op ) :
            reg_name( out ),
            operands( { in1, in2 } ),
            operation( op ),
            result( false )
    {
        resolve_operation( in1->get_result(), in2->get_result(), operation );
    }

    operation_node(
        const std::string& out,
        const bool& in1,
        const bool& in2,
        const std::string& op ) :
        reg_name( out ),
        operands( { nullptr, nullptr } ),
        operation( op ),
        result( false )
    {
        resolve_operation( in1, in2, operation );
    }

    const std::string& get_reg_name( void ) const
    {
        return reg_name;
    }

    const std::shared_ptr<operation_node> get_operand( size_t idx ) const
    {
        if( idx < 2 )
        {
            return operands[ idx ];
        }

        __debugbreak(); // out of bounds
        return operands[ 0 ];
    }

    bool get_result( void ) const
    {
        return result;
    }

private:
    void resolve_operation( const bool in1, const bool in2, const std::string& op )
    {
        if( op == "AND" )
        {
            result = ( in1 && in2 );
        }
        else if( op == "XOR" )
        {
            result = ( in1 && !in2 ) || ( !in1 && in2 );
        }
        else if( op == "OR" )
        {
            result = ( in1 || in2 );
        }
        else if( op == "ASSIGN" )
        {
            if( in1 != in2 )
            {
                __debugbreak();
            }

            result = in1;
        }
        else
        {
            // Unknown operation
            __debugbreak();
        }
    }

    std::string reg_name;
    std::array<std::shared_ptr<operation_node>, 2> operands;
    std::string operation;
    bool result;
};

static bool read_input(
    std::map<std::string, bool>& registers,
    std::map<std::string, std::tuple<std::string, std::string, std::string>>& operations
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Logic_full.txt");

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

static std::shared_ptr<operation_node> build_dependency_tree(
    const std::string& reg,
    const std::map<std::string, std::tuple<std::string, std::string, std::string>>& operations,
    std::map<std::string, bool>& registers
)
{
    std::string operand1 = std::get<0>( operations.at( reg ) );
    std::string operand2 = std::get<1>( operations.at( reg ) );

    if( 'x' == operand1[ 0 ] || 'y' == operand1[ 0 ] )
    {
        if( 'x' != operand2[ 0 ] && 'y' != operand2[ 0 ] )
        {
            __debugbreak();
        }

        // Bottom of the dependency tree, can resolve this node
        std::shared_ptr<operation_node> static_reg1(new operation_node(
                operand1,
                registers.at( operand1 ),
                registers.at( operand1 ),
                "ASSIGN"
            )
        );

        std::shared_ptr<operation_node> static_reg2(new operation_node(
                operand2,
                registers.at( operand2 ),
                registers.at( operand2 ),
                "ASSIGN"
            )
        );

        return std::make_shared<operation_node>( reg, static_reg1, static_reg2, std::get<2>( operations.at( reg ) ) );
    }
    else
    {
        return std::shared_ptr<operation_node>( new operation_node(
                reg,
                build_dependency_tree( operand1, operations, registers ),
                build_dependency_tree( operand2, operations, registers ),
                std::get<2>( operations.at( reg ) )
            )
        );
    }
}

static uint64_t calculate_z(
    std::vector<std::shared_ptr<operation_node>>& z_dep_trees
)
{
    uint64_t z_value = 0;

    for( auto& z_reg : z_dep_trees )
    {
        uint32_t shift = std::stoul( z_reg->get_reg_name().substr( 1, 2 ) );
        z_value |= ( static_cast<uint64_t>( z_reg->get_result() ? 1 : 0 ) << shift );
    }

    return z_value;
}

int main()
{
    std::map<std::string, bool> registers;
    std::map<std::string, std::tuple<std::string, std::string, std::string>> operations;
    if( !read_input( registers, operations ) )
    {
        return -1;
    }

    std::vector<std::shared_ptr<operation_node>> z_dep_trees;
    uint32_t z_idx = 0;
    while( 1 )
    {
        std::stringstream z_reg;
        z_reg << "z" << std::setfill('0') << std::setw( 2 ) << z_idx;
        if( !operations.contains( z_reg.str() ) )
        {
            break;
        }

        std::shared_ptr<operation_node> reg_tree =
            build_dependency_tree( z_reg.str(), operations, registers );

        z_dep_trees.emplace_back( std::move( reg_tree ) );

        ++z_idx;
    }

    uint64_t output = calculate_z( z_dep_trees );
    std::cout << "The decimal output is " << std::to_string( output ) << "\n";
}
