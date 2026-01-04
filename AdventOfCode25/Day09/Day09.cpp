// Day09.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "vec2.hpp"

static vec2 split( const std::string& str )
{
    std::stringstream ss( str );
    std::string token;
    std::array<int64_t, 2> values{ 0, 0 };

    size_t idx = 0;
    while( std::getline( ss, token, ',' ) )
    {
        int64_t value = std::stoll( token );
        if( 0 != value )
        {
            values[ idx++ ] = value;
        }
    }

    return { values[ 0 ], values[ 1 ] };
}

static bool read_input(
    const std::string& filename,
    std::vector<vec2>& input )
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

        input.emplace_back( split( line ) );
    }

    // Step 3: return success or failure.
    return true;
}

static void calculate_areas(
    const std::vector<vec2>& input,
    area_map_t& areas
)
{
    for( uint16_t start = 0; start < input.size() - 1; ++start )
    {
        for( uint16_t end = start + 1; end < input.size(); ++end )
        {
            double area = input[ start ].area( input[ end ] );
            std::pair<uint16_t, uint16_t> nodes = std::make_pair( start, end );

            areas.emplace( std::make_pair( area, nodes ) );
        }
    }
}

static bool check_segments_intersect(
    const vec2& a,
    const vec2& b,
    const vec2& o,
    const vec2& d,
    const bool want_hard_intersect
)
{
    vec2 ortho( -d.y, d.x );
    vec2 a_to_o( o - a );
    vec2 a_to_b( b - a );

    double denom = a_to_b.dot( ortho );
    if( 0.0 == denom )
    {
        // Ray and segment are parallel.
        return false;
    }

    double t1 = a_to_b.cross( a_to_o ) / denom;
    double t2 = a_to_o.dot( ortho ) / denom;

    bool t1_passed = want_hard_intersect ?
        ( 0.0 <= t1 ) && ( t1 <= 1.0 ) :
        0.0 <= t1;

    return t1_passed && ( 0.0 <= t2 ) && ( t2 <= 1.0 );
}

static bool validate_areas(
    const std::vector<vec2>& input,
    area_map_t& areas
)
{
    area_map_t::iterator area = areas.begin();
    while( areas.end() != area )
    {
        const auto& nodes = ( *area ).second;
        const vec2& v1 = input[ nodes.first ];
        const vec2& v2 = input[ nodes.second ];

        // First, check if any of the four sides of the rectangle cross a
        // polygon boundary.
        std::array<vec2, 4> vertices{{
            { std::min( v1.x, v2.x ) + 0.5, std::min( v1.y, v2.y ) + 0.5 },
            { std::min( v1.x, v2.x ) + 0.5, std::max( v1.y, v2.y ) - 0.5 },
            { std::max( v1.x, v2.x ) - 0.5, std::max( v1.y, v2.y ) - 0.5 },
            { std::max( v1.x, v2.x ) - 0.5, std::min( v1.y, v2.y ) + 0.5 }
        }};

        bool is_valid = true;
        for( size_t v1 = 0; is_valid && v1 < vertices.size(); ++v1 )
        {
            size_t v2 = ( v1 + 1 ) % vertices.size();

            for( size_t i = 0; is_valid && i < input.size(); ++i )
            {
                size_t j = ( i + 1 ) % input.size();
                if( check_segments_intersect(
                    input[ i ], input[ j ],
                    vertices[ v1 ], vertices[ v2 ] - vertices[ v1 ],
                    true )
                  )
                {
                    is_valid = false;
                }
            }
        }

        if( is_valid )
        {
            // Rectangle does not cross any boundaries of the polygon.
            // Now do a ray cast and count how many boundaries are crossed.
            // Since the rectangle is either fully enclosed by the polygon or
            // fully outside, the ray can be cast in any direction.
            vec2 o{ ( v1.x + v2.x ) / 2.0, ( v1.y + v2.y ) / 2.0 };
            o.x += ( o.x == static_cast<int64_t>( o.x ) ) ? 0.5 : 0.0;
            o.y += ( o.y == static_cast<int64_t>( o.y ) ) ? 0.5 : 0.0;

            size_t intersections = 0;
            for( size_t i = 0; is_valid && i < input.size(); ++i )
            {
                size_t j = ( i + 1 ) % input.size();
                if( check_segments_intersect(
                    input[ i ], input[ j ],
                    o, { 0.0, -1.0 },
                    false )
                    )
                {
                    ++intersections;
                }
            }
            if( 0 == intersections % 2 )
            {
                is_valid = false;
            }
        }

        if( !is_valid )
        {
            area = areas.erase( area );
        }
        else
        {
            // Found the largest valid area.
            return true;
        }
    }

    return false;
}

int main()
{
#if 1
    static const std::string filename( ".\\Data\\Input_test.txt" );
#else
    static const std::string filename( ".\\Data\\Input.txt" );
#endif

    std::vector<vec2> input;
    if( !read_input( filename, input ) )
    {
        return -1;
    }

    area_map_t areas;
    calculate_areas( input, areas );
    std::cout << "The largest area is ";
    std::cout << static_cast<int64_t>( ( *areas.begin() ).first );
    std::cout << std::endl;

    if( validate_areas( input, areas ) )
    {
        std::cout << "The largest valid area is ";
        std::cout << static_cast<int64_t>( ( *areas.begin() ).first );
        std::cout << std::endl;
    }
    else
    {
        std::cout << "No valid area found." << std::endl;
    }
}