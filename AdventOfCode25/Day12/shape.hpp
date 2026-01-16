#pragma once

#include <string>
#include <vector>

class shape
{
public:
    shape()
        : m_x( 0 )
        , m_y( 0 )
        , m_fa( 0 )
        , m_pa( 0 )
    {
    };

    void set_data( const std::array<std::string, 3>& raw )
    {
        m_layout = raw;
        m_x = m_layout[ 0 ].size();
        m_y = m_layout.size();

        m_fa = m_x * m_y;

        for( const auto& row : m_layout )
        {
            for( const auto& col : row )
            {
                m_pa += ( col == '#' ) ? 1 : 0;
            }
        }
    }

    size_t get_footprint_area( void ) const
    {
        return m_fa;
    }

    size_t get_perfect_area( void ) const
    {
        return m_pa;
    }

private:
    size_t m_x;
    size_t m_y;
    size_t m_fa;
    size_t m_pa;

    std::array<std::string, 3> m_layout;
};