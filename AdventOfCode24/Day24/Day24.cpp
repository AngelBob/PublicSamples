// Day24.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

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

    inline OP get_op( void ) const
    {
        return op;
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

    struct find_input
    {
    public:
        find_input(
            const std::shared_ptr<reg_n> r1,
            const std::shared_ptr<reg_n> r2,
            const gate_n::OP op ) :
            ab{ r1, r2 },
            op( op )
        {
        }

        inline bool operator()( const std::pair<size_t, std::shared_ptr<gate_n>>& g )
        {
            std::shared_ptr<reg_n> r1 = g.second->get_a_reg();
            std::shared_ptr<reg_n> r2 = g.second->get_b_reg();
            gate_n::OP g_op = g.second->get_op();

            return ( ( ( ab[ 0 ] == r1 && ab[ 1 ] == r2 ) ||
                       ( ab[ 0 ] == r2 && ab[ 1 ] == r1 ) ) &&
                     op == g_op );
        }

    private:
        std::array<std::shared_ptr<reg_n>, 2> ab;
        gate_n::OP op;
    };

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

static std::shared_ptr<reg_n> get_c0_reg(
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    const std::map<size_t, std::shared_ptr<gate_n>>& gates,
    std::vector<size_t>& swaps
)
{
    // First step - validate the "half-adder" at x00 / y00. The half-adder has
    // only two operations - an XOR to output z00 and an AND to create the
    // carry value to send to stage 1.
    // Find the gate with inputs x00 & y00 and an XOR operation.
    const size_t x_id = reg_n::calc_xyz_reg_id( 'x', 0 );
    const size_t y_id = reg_n::calc_xyz_reg_id( 'y', 0 );
    auto xor_gate_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_XOR
    ) );

    // Find the gate with inputs x00 & y00 and an AND operation.
    auto and_gate_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_AND
    ) );

    if( gates.end() == xor_gate_it || gates.end() == and_gate_it )
    {
        // This can't happen.
        __debugbreak();
    }

    // Check the output register
    std::shared_ptr<reg_n> c0{ nullptr };

    std::shared_ptr<reg_n> g_out = xor_gate_it->second->get_out_reg();
    const size_t z_id = reg_n::calc_xyz_reg_id( 'z', 0 );
    const size_t z_out = g_out->get_id();
    if( z_id == z_out )
    {
        c0 = and_gate_it->second->get_out_reg();
    }
    else
    {
        // Note the outputs on stage 0 have been swapped.
        swaps.emplace_back( and_gate_it->second->get_out_reg()->get_id() );
        swaps.emplace_back( z_out );

        c0 = r.at( z_out );
    }

    return c0;
}

static std::shared_ptr<reg_n> validate_adder(
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    const std::map<size_t, std::shared_ptr<gate_n>>& gates,
    const std::shared_ptr<reg_n> c_prev,
    const size_t stage,
    std::vector<size_t>& swaps
)
{
    // Each adder has the following operations:
    // gate0: x XOR y -> i0
    // gate1: c_prev XOR i0 -> z
    // gate2: x AND y -> i1
    // gate3: c_prev AND i1 -> i2
    // gate4: i1 OR i2 -> c

    // Validate that all of the output registers are properly feeding the input
    // chain of operations.
    const size_t x_id = reg_n::calc_xyz_reg_id( 'x', stage );
    const size_t y_id = reg_n::calc_xyz_reg_id( 'y', stage );
    const size_t z_id = reg_n::calc_xyz_reg_id( 'z', stage );

    std::shared_ptr<reg_n> z{ nullptr };
    std::shared_ptr<reg_n> i0{ nullptr };
    std::shared_ptr<reg_n> i1{ nullptr };
    std::shared_ptr<reg_n> i2{ nullptr };
    std::shared_ptr<reg_n> c{ nullptr };

    std::vector<std::shared_ptr<reg_n>> this_swap;

    const auto g0_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_XOR
    ) );
    i0 = g0_it->second->get_out_reg();

    const auto g1_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        c_prev,
        i0,
        gate_n::OP::OP_XOR
    ) );
    if( gates.end() != g1_it )
    {
        z = g1_it->second->get_out_reg();
    }
    else
    {
        // No z out that takes these inputs, find the z out gate. The current
        // z-out register should be swapped with the register in the z-out gate
        // that is not the previous carry register.
        std::shared_ptr<gate_n> z_g = gates.at( z_id );
        if( c_prev != z_g->get_a_reg() )
        {
            this_swap.emplace_back( z_g->get_a_reg() );
            this_swap.emplace_back( i0 );
            i0 = z_g->get_a_reg();
        }
        else
        {
            this_swap.emplace_back( z_g->get_b_reg() );
            this_swap.emplace_back( i0 );
            i0 = z_g->get_b_reg();
        }

        z = r.at( z_id );
    }

    const size_t z_out = z->get_id();
    if( z_id != z_out )
    {
        // The z output has been swapped.
        this_swap.emplace_back( r.at( z_id ) );
        this_swap.emplace_back( z );
    }

    const auto g2_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_AND
    ) );
    if( gates.end() != g2_it )
    {
        i1 = g2_it->second->get_out_reg();
        if( this_swap.size() )
        {
            if( i1 == this_swap.back() )
            {
                i1 = this_swap.front();
            }
            else if( i1 == this_swap.front() )
            {
                i1 = this_swap.back();
            }
        }
    }

    const auto g3_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        c_prev,
        i0,
        gate_n::OP::OP_AND
    ) );
    if( gates.end() != g3_it )
    {
        i2 = g3_it->second->get_out_reg();
        if( this_swap.size() )
        {
            if( i2 == this_swap.back() )
            {
                i2 = this_swap.front();
            }
            else if( i2 == this_swap.front() )
            {
                i2 = this_swap.back();
            }
        }
    }

    const auto c_it = std::find_if( gates.begin(), gates.end(), gate_n::find_input(
        i1,
        i2,
        gate_n::OP::OP_OR
    ) );
    if( gates.end() != c_it )
    {
        c = c_it->second->get_out_reg();
        if( this_swap.size() )
        {
            if( c == this_swap.back() )
            {
                c = this_swap.front();
            }
            else if( c == this_swap.front() )
            {
                c = this_swap.back();
            }
        }
    }

    for( const auto& swap : this_swap )
    {
        swaps.emplace_back( swap->get_id() );
    }

    return c;
}

static void validate_adders(
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    const std::map<size_t, std::shared_ptr<gate_n>>& gates,
    const size_t max_stage,
    std::vector<size_t>& swaps
)
{
    // Validate each stage's adder logic, prime the validation with the stage 0
    // carry register.
    std::shared_ptr<reg_n> carry_prev = get_c0_reg( r, gates, swaps );
    for( size_t stage = 1; stage < max_stage; ++stage )
    {
        carry_prev = validate_adder( r, gates, carry_prev, stage, swaps );
    }
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

    // Validate the adders.
    std::vector<size_t> swaps;
    validate_adders( r, gates, z_idx - 1, swaps );

    // Convert the swap register ids to names
    if( swaps.size() )
    {
        std::vector<std::string> swaps_s;
        for( const size_t r_id : swaps )
        {
            swaps_s.emplace_back( r.at( r_id )->get_out_symbol() );
        }

        // Sort and output a comma separated list of all the swaps.
        std::sort( swaps_s.begin(), swaps_s.end() );
        for( size_t idx = 0; idx < swaps_s.size() - 1; ++idx )
        {
            std::cout << swaps_s[ idx ] << ",";
        }
        std::cout << swaps_s[ swaps_s.size() - 1 ] << std::endl;
    }
    else
    {
        std::cout << "No swaps found." << std::endl;
    }
}
