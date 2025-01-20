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

    inline void set_a_reg( std::shared_ptr<reg_n> r1 )
    {
        ab[ 0 ] = r1;
    }

    inline const std::string& get_b_symbol( void ) const
    {
        return ab[ 1 ]->get_out_symbol();
    }

    inline std::shared_ptr<reg_n> get_b_reg( void ) const
    {
        return ab[ 1 ];
    }

    inline void set_b_reg( std::shared_ptr<reg_n> r2 )
    {
        ab[ 1 ] = r2;
    }

    inline const std::string& get_out_symbol( void ) const
    {
        return out->get_out_symbol();
    }

    inline std::shared_ptr<reg_n> get_out_reg( void ) const
    {
        return out;
    }

    inline void set_out_reg( std::shared_ptr<reg_n> o )
    {
        out = o;
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
    std::map<size_t, std::tuple<std::string, std::string, std::string, gate_n::OP>>& gates,
    size_t& max_stage
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Logic_full.txt");

    // Step 2: read each line and insert registers and gates into the map.
    bool have_registers = false;

    size_t x_max = 0;
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
            if( x_max < idx )
            {
                x_max = idx;
            }
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

    max_stage = x_max;

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

static void do_register_swap(
    const bool fix_stage,
    const std::vector<std::shared_ptr<reg_n>> swaps,
    std::shared_ptr<reg_n>& r,
    std::shared_ptr<gate_n>& g
)
{
    if( r == swaps.back() )
    {
        r = swaps.front();
        if( fix_stage )
        {
            g->set_out_reg( r );
        }
    }
    else if( r == swaps.front() )
    {
        r = swaps.back();
        if( fix_stage )
        {
            g->set_out_reg( r );
        }
    }
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

    const size_t xor_out_id = xor_gate_it->second->get_out_reg()->get_id();
    const size_t z_id = reg_n::calc_xyz_reg_id( 'z', 0 );
    if( z_id != xor_out_id )
    {
        // Note the outputs on stage 0 have been swapped.
        swaps.emplace_back( xor_gate_it->second->get_out_reg()->get_id() );
        swaps.emplace_back( z_id );

        // Do the swap
        and_gate_it->second->set_out_reg( xor_gate_it->second->get_out_reg() );
        xor_gate_it->second->set_out_reg( r.at( z_id ) );
    }

    return and_gate_it->second->get_out_reg();
}

static std::array<std::shared_ptr<gate_n>, 5> get_stage_gates(
    const size_t stage,
    const bool fix_stage,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::shared_ptr<reg_n>& c_prev,
    std::map<size_t, std::shared_ptr<gate_n>>& g,
    std::vector<size_t>& swaps
)
{
    const size_t x_id = reg_n::calc_xyz_reg_id( 'x', stage );
    const size_t y_id = reg_n::calc_xyz_reg_id( 'y', stage );
    const size_t z_id = reg_n::calc_xyz_reg_id( 'z', stage );

    std::vector<std::shared_ptr<reg_n>> this_swap;
    std::array<std::shared_ptr<gate_n>, 5> g_s;

    // Gate 0: a XOR b -> i0
    std::shared_ptr<reg_n> i0{ nullptr };
    auto g0_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_XOR
    ) );
    i0 = g0_it->second->get_out_reg();

    // Gate 1: carry_n-1 XOR i0 -> z_stage
    std::shared_ptr<reg_n> z{ nullptr };
    auto g1_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
        c_prev,
        i0,
        gate_n::OP::OP_XOR
    ) );
    if( g.end() != g1_it )
    {
        z = g1_it->second->get_out_reg();
    }
    else
    {
        // No z out that takes these inputs, find the z out gate. The current
        // z-out register should be swapped with the register in the z-out gate
        // that is not the previous carry register.
        std::shared_ptr<gate_n> z_g = g.at( z_id );
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

        // Try the search again, it should work this time.
        g1_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
            c_prev,
            i0,
            gate_n::OP::OP_XOR
        ) );

        if( fix_stage )
        {
            g0_it->second->set_out_reg( i0 );
        }
        z = r.at( z_id );
    }

    const size_t z_out = z->get_id();
    if( z_id != z_out )
    {
        // The z output has been swapped.
        this_swap.emplace_back( r.at( z_id ) );
        this_swap.emplace_back( z );

        if( fix_stage )
        {
            g1_it->second->set_out_reg( r.at( z_id ) );
        }
    }

    // Gate 3: x AND y -> i1
    std::shared_ptr<reg_n> i1{ nullptr };
    auto g2_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
        r.at( x_id ),
        r.at( y_id ),
        gate_n::OP::OP_AND
    ) );
    if( g.end() != g2_it )
    {
        i1 = g2_it->second->get_out_reg();
        if( this_swap.size() )
        {
            do_register_swap( fix_stage, this_swap, i1, g2_it->second );
        }
    }

    // Gate 4: carry_n-1 AND i0 -> i2
    std::shared_ptr<reg_n> i2{ nullptr };
    auto g3_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
        c_prev,
        i0,
        gate_n::OP::OP_AND
    ) );
    if( g.end() != g3_it )
    {
        i2 = g3_it->second->get_out_reg();
        if( this_swap.size() )
        {
            do_register_swap( fix_stage, this_swap, i2, g3_it->second );
        }
    }

    // Gate 5: i1 OR i2 -> carry_stage
    std::shared_ptr<reg_n> c{ nullptr };
    auto c_it = std::find_if( g.begin(), g.end(), gate_n::find_input(
        i1,
        i2,
        gate_n::OP::OP_OR
    ) );
    if( g.end() != c_it )
    {
        c = c_it->second->get_out_reg();
        if( this_swap.size() )
        {
            do_register_swap( fix_stage, this_swap, c, c_it->second );
        }
    }

    if( 0 != stage )
    {
        g_s[ 0 ] = g0_it->second;
        g_s[ 1 ] = g1_it->second;
        g_s[ 2 ] = g2_it->second;
        g_s[ 3 ] = g3_it->second;
        g_s[ 4 ] = c_it->second;

        c_prev = c;
    }
    else
    {
        g_s[ 1 ] = g0_it->second;
        g_s[ 4 ] = g2_it->second;

        c_prev = g2_it->second->get_out_reg();
    }

    for( const auto& swap : this_swap )
    {
        swaps.emplace_back( swap->get_id() );
    }

    return g_s;
}

static void execute_adder(
    const size_t idx,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::shared_ptr<reg_n>& c_prev,
    std::map<size_t, std::shared_ptr<gate_n>>& gates,
    std::vector<size_t>& swaps
)
{
    std::array<std::shared_ptr<gate_n>, 5> g_s =
        get_stage_gates( idx, false, r, c_prev, gates, swaps );

    // Execute the gates
    if( 0 != idx )
    {
        g_s[ 0 ]->execute();
        g_s[ 2 ]->execute();
        g_s[ 1 ]->execute();
        g_s[ 3 ]->execute();
        g_s[ 4 ]->execute();
    }
    else
    {
        g_s[ 1 ]->execute();
        g_s[ 4 ]->execute();
    }
}

static uint64_t execute_adders(
    const size_t max_stage,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::map<size_t, std::shared_ptr<gate_n>>& gates,
    std::vector<size_t>& swaps
)
{
    // Validate each stage's adder logic, prime the validation with the stage 0
    // carry register.
    uint64_t r_long;
    uint64_t output = 0;
    std::shared_ptr<reg_n> carry_prev = get_c0_reg( r, gates, swaps );
    for( size_t idx = 0; idx <= max_stage; ++idx )
    {
        // Execute the adder top-down.
        execute_adder( idx, r, carry_prev, gates, swaps );

        // Get the z register result
        std::shared_ptr<reg_n> z = r.at( reg_n::calc_xyz_reg_id( 'z', idx ) );
        r_long = ( z->get_value() ) ? 1 : 0;
        output |= ( r_long << idx );
    }
    r_long = carry_prev->get_value() ? 1 : 0;
    output |= ( r_long  << ( max_stage + 1 ) );

    return output;
}

static std::shared_ptr<reg_n> validate_adder(
    const size_t stage,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::map<size_t, std::shared_ptr<gate_n>>& gates,
    std::shared_ptr<reg_n>& c_prev,
    std::vector<size_t>& swaps
)
{
    std::array<std::shared_ptr<gate_n>, 5> g_s =
        get_stage_gates( stage, true, r, c_prev, gates, swaps );

    return g_s[ 4 ]->get_out_reg();
}

static void validate_adders(
    const size_t max_stage,
    const std::map<size_t, std::shared_ptr<reg_n>>& r,
    std::map<size_t, std::shared_ptr<gate_n>>& gates,
    std::vector<size_t>& swaps
)
{
    // Validate each stage's adder logic, prime the validation with the stage 0
    // carry register.
    std::shared_ptr<reg_n> carry_prev = get_c0_reg( r, gates, swaps );
    for( size_t stage = 1; stage < max_stage; ++stage )
    {
        carry_prev = validate_adder( stage, r, gates, carry_prev, swaps );
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
    size_t max_stage;
    if( !read_input( r, ops, max_stage ) )
    {
        return -1;
    }

    // Have all of the the registers now, create the map of gate objects.
    std::map<size_t, std::shared_ptr<gate_n>> gates;
    build_gates( ops, r, gates );

    // Execute the ripple adder from LSB to MSB.
    std::vector<size_t> swaps;
    uint64_t output = execute_adders( max_stage, r, gates, swaps );
    std::cout << "The decimal output is " << std::to_string( output ) << "\n";

    // Validate the adders.
    swaps.clear();
    validate_adders( max_stage, r, gates, swaps );

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

        std::cout << "fixed the following registers:\n";
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

    swaps.clear();
    output = execute_adders( max_stage, r, gates, swaps );
    std::cout << "The decimal output after swaps is " << std::to_string( output ) << "\n";
}
