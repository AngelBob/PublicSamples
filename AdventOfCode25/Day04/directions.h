#pragma once

#include <array>
#include <string>

class compass
{
public:
    enum DIRECTIONS
    {
        DIR_START = 0,
        DIR_NORTH = DIR_START,
        DIR_SOUTH,
        DIR_EAST,
        DIR_WEST,
        DIR_CARDINAL_END,

        DIR_NORTHWEST = DIR_CARDINAL_END,
        DIR_NORTHEAST,
        DIR_SOUTHEAST,
        DIR_SOUTHWEST,
        DIR_END
    };

    class compass_iterator
    {
    public:
        compass_iterator( const enum DIRECTIONS dir )
            : m_index( dir )
        {
        }

        compass_iterator& operator++( void )
        {
            m_index = static_cast<enum DIRECTIONS>( m_index + 1 );
            if( m_index > DIRECTIONS::DIR_END )
            {
                m_index = DIRECTIONS::DIR_END;
            }

            return *this;
        }

        bool operator!=( const compass_iterator& other ) const
        {
            return m_index != other.m_index;
        }

        enum DIRECTIONS operator*( void ) const
        {
            return m_index;
        }

    private:
        enum DIRECTIONS m_index;
    };
    using iterator = compass_iterator;

    static std::pair<int8_t, int8_t> get_xy_offset( const enum DIRECTIONS dir )
    {
        return { m_directions[ dir ].x_inc, m_directions[ dir ].y_inc };
    }

    static wchar_t get_direction_indicator( const enum DIRECTIONS dir )
    {
        return m_directions[ dir ].indicator;
    }

    static enum DIRECTIONS get_direction_from_indicator( const wchar_t indicator )
    {
        for( const auto& direction : m_directions )
        {
            if( direction.indicator == indicator )
            {
                return direction.dir;
            }
        }

        return DIRECTIONS::DIR_END;
    }

    compass_iterator& begin( void )
    {
        static compass_iterator iter( DIRECTIONS::DIR_START );
        return iter;
    }

    compass_iterator& end( void )
    {
        static compass_iterator iter( DIRECTIONS::DIR_END );
        return iter;
    }

private:
    struct compass_data
    {
        enum DIRECTIONS dir;
        int8_t          x_inc;
        int8_t          y_inc;
        const wchar_t   indicator;
        const wchar_t*  name;
    };

    static constexpr std::array<struct compass_data, DIRECTIONS::DIR_END> m_directions
    {{
        { DIRECTIONS::DIR_NORTH,      0, -1, L'^', L"NORTH" },
        { DIRECTIONS::DIR_SOUTH,      0,  1, L'v', L"SOUTH" },
        { DIRECTIONS::DIR_EAST,       1,  0, L'<', L"EAST" },
        { DIRECTIONS::DIR_WEST,      -1,  0, L'>', L"WEST" },
        { DIRECTIONS::DIR_NORTHWEST, -1, -1, L'`', L"NORTH WEST" },
        { DIRECTIONS::DIR_NORTHEAST,  1, -1, L'ˊ', L"NORTH EAST" },
        { DIRECTIONS::DIR_SOUTHEAST,  1,  1, L'ˎ', L"SOUTH EAST" },
        { DIRECTIONS::DIR_SOUTHWEST, -1,  1, L',', L"SOUTH WEST" },
    }};
};
