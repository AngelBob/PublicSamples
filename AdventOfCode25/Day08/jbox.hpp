#pragma once
#include <tuple>

class jbox
{
public:
    using location = std::tuple<uint64_t, uint64_t, uint64_t>;

    jbox() = delete;
    jbox( size_t input_size, location& ijk )
        : xyz( std::move( ijk ) )
        , idx( input_size )
    {
    }

    uint64_t distance2( const jbox& other ) const
    {
        uint64_t x2 = std::get<0>( xyz ) - std::get<0>( other.xyz );
        uint64_t y2 = std::get<1>( xyz ) - std::get<1>( other.xyz );
        uint64_t z2 = std::get<2>( xyz ) - std::get<2>( other.xyz );

        x2 *= x2;
        y2 *= y2;
        z2 *= z2;

        return ( x2 + y2 + z2 );
    }

private:
    location xyz;
    size_t idx;
};

using distance_map = std::map<size_t, std::pair<uint16_t, uint16_t>>;