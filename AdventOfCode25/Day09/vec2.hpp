#pragma once

class vec2
{
public:
    vec2() = delete;
    vec2( const int64_t _x, const int64_t _y )
        : x( static_cast<double>( _x ) )
        , y( static_cast<double>( _y ) )
    {
    }

    vec2( const double _x, const double _y )
        : x( _x )
        , y( _y )
    {
    }

    double area( const vec2& other ) const
    {
        double dx = std::abs( x - other.x ) + 1.0;
        double dy = std::abs( y - other.y ) + 1.0;

        return ( dx * dy );
    }

    double cross( const vec2& other ) const
    {
        return ( ( x * other.y ) - ( y * other.x ) );
    }

    double dot( const vec2& other ) const
    {
        return ( ( x * other.x ) + ( y * other.y ) );
    }

    vec2 operator+( const vec2& other ) const
    {
        return { ( x - other.x ), ( y - other.y ) };
    }

    vec2 operator-( const vec2& other ) const
    {
        return { ( x - other.x ), ( y - other.y ) };
    }

    double x;
    double y;
};

using area_map_t = std::multimap<double, std::pair<uint16_t, uint16_t>, std::greater<double>>;