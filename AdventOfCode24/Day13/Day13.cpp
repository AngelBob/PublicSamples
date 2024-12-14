// Day13.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <vector>

static bool read_input(
    std::vector<std::array<std::pair<uint64_t, uint64_t>, 3>>& machine_data
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Machines.txt" );

    // Step 2: read the single line of input.
    static const std::regex buttons( "([AB]): X\\+([0-9]+), Y\\+([0-9]+)" );
    static const std::regex prizes( "X=([0-9]+), Y=([0-9]+)" );

    std::pair<uint64_t, uint64_t> buttonA;
    std::pair<uint64_t, uint64_t> buttonB;

    std::string line;
    while( std::getline( file, line ) )
    {
        // Do a regex match to extract the machine data from the line.
        std::smatch reg_match;
        if( std::regex_search( line.cbegin(), line.cend(), reg_match, buttons ) )
        {
            if( "A" == reg_match[ 1 ] )
            {
                buttonA.first = stoul( reg_match[ 2 ] );
                buttonA.second = stoul( reg_match[ 3 ] );
            }
            else
            {
                buttonB.first = stoul( reg_match[ 2 ] );
                buttonB.second = stoul( reg_match[ 3 ] );
            }
        }
        else if( std::regex_search( line.cbegin(), line.cend(), reg_match, prizes ) )
        {
            machine_data.emplace_back(
                std::array<std::pair<uint64_t, uint64_t>, 3>{{
                    buttonA,
                    buttonB,
                    std::make_pair( stoul( reg_match[ 1 ] ), stoul( reg_match[ 2 ] ) )
                }} );
        }
    }

    // Step 3: return success or failure.
    return ( 0 != machine_data.size() );
}

static const uint64_t cost_A = 3;
static const uint64_t cost_B = 1;

static uint64_t find_path(
    std::array<std::pair<uint64_t, uint64_t>, 3> machine,
    bool is_way_off
)
{
    // Solve the 2x2 matrix
    // [x,y] = A^-1 * [px,py]

    // Find the inverse of the 2x2 matrix. Inverse is 1/determinant * adjoint M
    std::array<std::array<double, 2>, 2> M{ {
        {{ static_cast<double>( machine[0].first ),
           static_cast<double>( machine[1].first ) }},
        {{ static_cast<double>( machine[0].second ),
           static_cast<double>( machine[1].second ) }}
    }};
    double determinant = ( M[0][0] * M[1][1] ) - ( M[1][0] * M[0][1] );

    // Create the adjoint M
    double temp = M[0][0];
    M[0][0] =  M[1][1];
    M[0][1] = -M[0][1];
    M[1][0] = -M[1][0];
    M[1][1] =  temp;

    // Do the multiplication to find the inverse
    M[0][0] /= determinant;
    M[0][1] /= determinant;
    M[1][0] /= determinant;
    M[1][1] /= determinant;

    if( is_way_off )
    {
        machine[2].first  += 10000000000000;
        machine[2].second += 10000000000000;
    }

    // Multiply the matrix by the prize location to find the X,Y
    double prize_x = static_cast<double>( machine[2].first );
    double prize_y = static_cast<double>( machine[2].second );

    double a = ( M[0][0] * prize_x ) + ( M[0][1] * prize_y );
    double b = ( M[1][0] * prize_x ) + ( M[1][1] * prize_y );

    // Convert the floating point answer to integer and verify that it is a
    // valid result
    uint64_t A = std::llround( a );
    uint64_t B = std::llround( b );
    uint64_t cost = 0;

    if( ( ( A * machine[0].first )  + ( B * machine[1].first )  == machine[2].first  ) &&
        ( ( A * machine[0].second ) + ( B * machine[1].second ) == machine[2].second ) )
    {
        cost = ( ( A * cost_A ) + ( B * cost_B ) );
    }

    return cost;
}

static uint64_t find_optimized_cost(
    const std::vector<std::array<std::pair<uint64_t, uint64_t>, 3>>& machine_data
)
{
    // Each machine moves the claw in a specific way, solve the equations to
    // determine the most efficient path, if there is one.
    uint64_t total_tokens = 0;
    for( const auto& this_machine : machine_data )
    {
        // Find all paths, if any.
        total_tokens += find_path( this_machine, true );
    }

    return total_tokens;
}
int main()
{
    std::vector<std::array<std::pair<uint64_t, uint64_t>, 3>> machine_data;
    if( !read_input( machine_data ) )
    {
        return -1;
    }

    uint64_t total_cost = find_optimized_cost( machine_data );
    std::cout << "The total cost for all possible prizes is " << total_cost << "\n";
}
