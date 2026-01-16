#pragma once
#include <array>
#include <sstream>
#include <string>

class layout
{
public:
    layout() = delete;
    layout( const std::string& l_text )
    {
        std::stringstream ss( l_text );
        std::string token;

        std::getline( ss, token, 'x' );
        m_x = std::stoi( token );

        std::getline( ss, token, ':' );
        m_y = std::stoi( token );

        m_a = m_x * m_y;

        size_t idx = 0;
        while( ss >> token )
        {
            m_shapes[ idx++ ] = std::stoi( token );
        }
    }

    const size_t get_area( void ) const
    {
        return m_a;
    }

    const std::array<size_t, 6>& get_shapes( void ) const
    {
        return m_shapes;
    }

private:
    size_t m_x;
    size_t m_y;
    size_t m_a;

    std::array<size_t, 6> m_shapes;
};