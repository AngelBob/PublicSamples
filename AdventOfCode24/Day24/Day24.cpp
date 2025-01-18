// Day24.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>

class reg_n
{
public:
    reg_n() = delete;

    reg_n(
        size_t id,
        bool is_set,
        bool value,
        const std::string& name
    ) :
        id( id ),
        set( is_set ),
        val( value ),
        reg( name )
    {
    }

    inline size_t get_id( void ) const
    {
        return id;
    }

    inline const std::string& get_out_symbol( void ) const
    {
        return reg;
    }

    inline void set_value( bool v )
    {
        if( set && ( val != v ) )
        {
            std::cout << reg << " reset." << std::endl;
            __debugbreak();
        }

        val = v;
        set = true;
    }

    inline bool get_value( void ) const
    {
        if( !set )
        {
            __debugbreak();
        }

        return val;
    }

    static size_t calc_register_id( const std::string& reg_name )
    {
        size_t shift = 0;
        size_t id = 0;
        for( size_t idx = 0; idx < reg_name.size(); ++idx, shift += 8 )
        {
            char c = reg_name[ idx ];
            if( 'a' <= c && c <= 'z' )
            {
                c -= 'a';
                c += 10; /* ensure 0-9 sort before a-z */
            }
            else if( '0' <= c && c <= '9' )
            {
                c -= '0';
            }

            id |= ( static_cast<size_t>( c ) << shift );
        }

        return id;
    }

    static size_t calc_xyz_reg_id( const char xyz, size_t idx )
    {
        size_t id = ( static_cast<size_t>( xyz ) - 'a' ) + 10;
        id |= ( ( idx / 10 ) << 8 );
        id |= ( ( idx % 10 ) << 16 );

        return id;
    }

private:
    std::string reg;
    size_t id{ 0 };
    bool set{ false };
    bool val{ false };
};

class gate_n
{
public:
    enum OP
    {
        OP_XOR = 0,
        OP_AND,
        OP_OR,
        OP_ASSIGN,
        OP_NONE = 0xFF
    };

    gate_n() = delete;

    gate_n(
        std::shared_ptr<reg_n> out,
        std::shared_ptr<reg_n> a,
        std::shared_ptr<reg_n> b,
        const OP op ) :
            out( out ),
            ab( { a, b } ),
            op( op )
    {
    }

    inline const std::string& get_a_symbol( void ) const
    {
        return ab[ 0 ]->get_out_symbol();
    }

    inline std::shared_ptr<reg_n> get_a_reg( void ) const
    {
        return ab[ 0 ];
    }

    inline const std::string& get_b_symbol( void ) const
    {
        return ab[ 1 ]->get_out_symbol();
    }

    inline std::shared_ptr<reg_n> get_b_reg( void ) const
    {
        return ab[ 1 ];
    }

    inline const std::string& get_out_symbol( void ) const
    {
        return out->get_out_symbol();
    }

    inline std::shared_ptr<reg_n> get_out_reg( void ) const
    {
        return out;
    }

    inline void execute( void )
    {
        resolve_gate();
    }

    inline bool get_result( void ) const
    {
        return out->get_value();
    }

    inline static OP resolve_op_string( const std::string& op )
    {
        OP resolved_op = OP::OP_NONE;
        if( "XOR" == op )
        {
            resolved_op = OP::OP_XOR;
        }
        else if( "AND" == op )
        {
            resolved_op = OP::OP_AND;
        }
        else if( "OR" == op )
        {
            resolved_op = OP::OP_OR;
        }
        else if( "ASSIGN" == op )
        {
            resolved_op = OP::OP_ASSIGN;
        }
        else
        {
            // Unknown operation
            __debugbreak();
        }

        return resolved_op;
    }

private:
    void resolve_gate( void )
    {
        // Ensure the register booleans are exactly 1 bit in size.
        uint8_t a_val = ab[ 0 ]->get_value() ? 0x1 : 0x0;
        uint8_t b_val = ab[ 1 ]->get_value() ? 0x1 : 0x0;
        uint8_t o;

        switch( op )
        {
        case OP::OP_XOR:
            o = ( a_val ^ b_val );
            break;

        case OP::OP_AND:
            o = ( a_val & b_val );
            break;

        case OP::OP_OR:
            o = ( a_val | b_val );
            break;

        case OP::OP_ASSIGN:
            if( a_val != b_val )
            {
                __debugbreak();
            }

            o = a_val ? 1 : 0;
            break;

        default:
            // Unknown operation
            __debugbreak();
        }

        out->set_value( 0 == o ? false : true );
    }

    std::shared_ptr<reg_n> out;
    std::array<std::shared_ptr<reg_n>, 2> ab;
    OP op;
};

static bool read_input(
    std::map<size_t, std::shared_ptr<reg_n>>& registers,
    std::map<size_t, std::tuple<std::string, std::string, std::string, gate_n::OP>>& gates
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Logic_full.txt");

    // Step 2: read each line and insert registers and gates into the map.
    bool have_registers = false;

    std::string line;
    while( std::getline( file, line ) )
    {
        if( 0 == line.length() )
        {
            have_registers = true;
            continue;
        }

        if( !have_registers )
        {
            std::string reg( line.substr( 0, line.find( ':' ) ) );
            size_t reg_id = reg_n::calc_register_id( reg );

            size_t idx = std::stoull( reg.substr( 1 ) );
            bool val = std::stoi( line.substr( line.find( ':' ) + 2, line.length() - 1 ) ) ? true : false;

            registers.emplace( std::make_pair( reg_id, std::make_shared<reg_n>( reg_id, true, val, reg ) ) );
        }
        else
        {
            std::string a( line.substr( 0, 3 ) );
            std::string b( line.substr( line.find( ' ', 4 ) + 1, 3 ) );

            size_t reg_a_id = reg_n::calc_register_id( a );
            size_t reg_b_id = reg_n::calc_register_id( b );

            registers.emplace( std::make_pair( reg_a_id, std::make_shared<reg_n>( reg_a_id, false, false, a ) ) );
            registers.emplace( std::make_pair( reg_b_id, std::make_shared<reg_n>( reg_b_id, false, false, b ) ) );

            std::string op( line.substr( 4, line.find( ' ', 4 ) - 4 ) );
            std::string out( line.substr( line.length() - 3, line.length() - 1 ) );

            size_t out_id = reg_n::calc_register_id( out );
            registers.emplace( std::make_pair( out_id, std::make_shared<reg_n>( out_id, false, false, out ) ) );

            gates.emplace( std::make_pair( out_id, std::make_tuple( a, b, out, gate_n::resolve_op_string( op ) ) ) );
        }
    }

    // Step 3: return success or failure.
    return( 0 != registers.size() && 0 != gates.size() );
}

static void build_gates(
    const std::map<size_t, std::tuple<std::string, std::string, std::string, gate_n::OP>>& op_descriptions,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::map<size_t, std::shared_ptr<gate_n>>& ops
)
{
    for( const auto& op_desc : op_descriptions )
    {
        size_t out_id = reg_n::calc_register_id( std::get<2>( op_desc.second ) );
        size_t a_id   = reg_n::calc_register_id( std::get<0>( op_desc.second ) );
        size_t b_id   = reg_n::calc_register_id( std::get<1>( op_desc.second ) );
        gate_n::OP x    = std::get<3>( op_desc.second );

        ops.emplace( std::make_pair( out_id, std::make_shared<gate_n>( r.at( out_id ), r.at( a_id ), r.at( b_id ), x ) ) );
    }
}

static void resolve_registers(
    const size_t reg,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::map<size_t, std::shared_ptr<gate_n>>& ops
)
{
    std::shared_ptr<reg_n> a = ops.at( reg )->get_a_reg();
    std::shared_ptr<reg_n> b = ops.at( reg )->get_b_reg();

    if( 'x' == a->get_out_symbol()[ 0 ] || 'y' == a->get_out_symbol()[ 0 ] )
    {
        if( 'x' != b->get_out_symbol()[ 0 ] && 'y' != b->get_out_symbol()[ 0 ] )
        {
            __debugbreak();
        }

        // Bottom of the dependency tree, start resolving bottoms-up.
    }
    else
    {
        resolve_registers( a->get_id(), r, ops );
        resolve_registers( b->get_id(), r, ops );
    }

    // All registers below should be fully resolved
    ops.at( reg )->execute();
}

int main()
{
    // Read input will create a full list of registers and resolve the values
    // for the input x & y. Can not fully resolve the gates at this time
    // due to ordering of register creation, so build a map of data per output
    // register.
    std::map<size_t, std::shared_ptr<reg_n>> r;
    std::map<size_t, std::tuple<std::string, std::string, std::string, gate_n::OP>> ops;
    if( !read_input( r, ops ) )
    {
        return -1;
    }

    // Have all of the the registers now, create the map of gate objects.
    std::map<size_t, std::shared_ptr<gate_n>> gates;
    build_gates( ops, r, gates );

    // Execute the ripple adder from LSB to MSB.
    // For each gate with a z register output, resolve the gates from
    // bottom to top.
    uint32_t z_idx = 0;
    uint64_t output = 0;
    while( 1 )
    {
        size_t z_id = reg_n::calc_xyz_reg_id( 'z', z_idx );
        if( !ops.contains( z_id ) )
        {
            break;
        }

        resolve_registers( z_id, r, gates );
        bool result = gates.at( z_id )->get_result();
        uint64_t r_long = result ? 1 : 0;
        output |= ( r_long << z_idx );

        ++z_idx;
    }

    std::cout << "The decimal output is " << std::to_string( output ) << "\n";
}
