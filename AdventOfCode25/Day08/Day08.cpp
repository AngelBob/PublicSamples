// Day08.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "jbox.hpp"
#include "circuit.hpp"

jbox::location split( const std::string& str )
{
    std::stringstream ss( str );
    std::string token;
    std::array<uint64_t, 3> values{ 0, 0, 0 };

    size_t idx = 0;
    while( std::getline( ss, token, ',' ) )
    {
        uint64_t value = std::stoll( token );
        if( 0 != value )
        {
            values[ idx++ ] = value;
        }
    }


    return jbox::location( values[ 0 ], values[ 1 ], values[ 2 ] );
}

static bool read_input( const std::string& filename, std::vector<std::shared_ptr<jbox>>& input )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( filename );

    // Step 2: read the lines and create the input data.
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );

        jbox::location xyz = split( line );
        input.emplace_back( std::make_shared<jbox>( input.size(), xyz ) );
    }

    // Step 3: return success or failure.
    return true;
}

static void calculate_distances(
    const std::vector<std::shared_ptr<jbox>>& input,
    distance_map& distances
)
{
    for( uint16_t start = 0; start < input.size() - 1; ++start )
    {
        for( uint16_t end = start + 1; end < input.size(); ++end )
        {
            uint64_t dist = input[ start ].get()->distance2( *( input[ end ].get() ) );
            std::pair<uint16_t, uint16_t> nodes = std::make_pair( start, end );

            distances.emplace( std::make_pair( dist, nodes ) );
        }
    }
}

static void make_connections(
    const std::vector<std::shared_ptr<jbox>>& input,
    const distance_map& distances,
    const size_t max_connections,
    circuit_map& circuits
)
{
    // The distances map automatically sorts the jbox pairs by distance.
    distance_map::const_iterator pos = distances.begin();

    for( size_t i = 0; i < max_connections; ++i )
    {
        const std::pair<uint16_t, uint16_t> n = ( *pos ).second;

        circuit_map::iterator c1 = std::find_if(
            circuits.begin(),
            circuits.end(),
            circuit::circuit_has_jbox( n.first )
        );

        circuit_map::iterator c2 = std::find_if(
            circuits.begin(),
            circuits.end(),
            circuit::circuit_has_jbox( n.second )
        );

        if( circuits.end() == c1 && circuits.end() == c2 )
        {
            // Neither jbox is in a circuit, create a new circuit.
            circuit new_circuit( n.first, n.second );
            circuits.insert( std::make_pair( 2, std::move( new_circuit ) ) );
        }
        else
        {
            circuit_map::iterator to_update = circuits.end();

            if( circuits.end() != c1 && circuits.end() == c2 )
            {
                // The first jbox is already in a circuit, add the second to the same
                // circuit.
                ( ( *c1 ).second ).insert_jbox( n.second );
                to_update = c1;
            }
            else if( circuits.end() == c1 && circuits.end() != c2 )
            {
                // The second jbox is already in a circuit, add the first to the same
                // circuit.
                ( *c2 ).second.insert_jbox( n.first );
                to_update = c2;
            }
            else if( c1 != c2 )
            {
                // Both jboxes already in a circuit, but they are different circuits.
                // Combine the two circuits.
                ( *c1 ).second.merge( ( *c2 ).second );
                circuits.erase( c2 );
                to_update = c1;
            }

            if( circuits.end() != to_update )
            {
                auto nh = circuits.extract( to_update );
                if( !nh.empty() )
                {
                    nh.key() = nh.mapped().get_size();
                    circuits.insert( std::move( nh ) );
                }
            }
        }

        ++pos;
    }
}

int main()
{
#if 1
    static const std::string filename( ".\\Data\\Input_test.txt" );
    static const size_t stop_at = 10;
#else
    static const std::string filename( ".\\Data\\Input.txt" );
    static const size_t stop_at = 1000;
#endif

    std::vector<std::shared_ptr<jbox>> input;
    if( !read_input( filename, input ) )
    {
        return -1;
    }

    // Build a map of distances between jboxes.
    distance_map distances;
    calculate_distances( input, distances );

    // Start building the circuits.
    circuit_map circuits;
    make_connections( input, distances, stop_at, circuits );

    // Multiply the sizes of the three largest circuits.
    uint64_t result = 1;
    circuit_map::iterator pos = circuits.begin();
    for( size_t i = 0; i < 3; ++i )
    {
        result *= pos->first;
        ++pos;
    }
    std::cout << "The three largest circuits result is " << result;
    std::cout << " after " << stop_at << " rounds." << std::endl;
}
