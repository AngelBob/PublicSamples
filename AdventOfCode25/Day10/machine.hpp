#pragma once
#include <algorithm>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "combinationanator.hpp"

class machine
{
public:
    machine() = delete;
    machine( const std::string& machine )
    {
        parse_machine( machine );

        // Build all of the button press combinations.
        std::vector<uint32_t> indices( m_buttons.size() );
        std::iota( indices.begin(), indices.end(), 0 );
        m_bcombinations.init( indices );
    }

    size_t set_lights( void )
    {
        // Try all of the combinations...
        std::vector<uint32_t> combo_result( m_lights.size() );
        for( const auto& combo : m_bcombinations )
        {
            std::fill( combo_result.begin(), combo_result.end(), 0 );
            for( const auto& idx : combo )
            {
                for( size_t bit = 0; bit < m_lights.size(); ++bit )
                {
                    combo_result[ bit ] ^= m_buttons[ idx ][ bit ];
                }
            }

            if( combo_result == m_lights )
            {
                return combo.size();
            }
        }

        // Should never reach here.
        return 0;
    }

private:
    void parse_machine( const std::string& machine )
    {
        std::stringstream ss( machine );

        std::vector<std::string> tokens;
        std::string token;
        while( ss >> token )
        {
            tokens.emplace_back( std::move( token ) );
        }

        // Lights are the first token.
        parse_lights( tokens.front() );

        // Joltages are the last token.
        parse_joltage( tokens.back(), m_lights.size() );

        // Buttons are everything in between.
        for( size_t i = 1; i < tokens.size() - 1; ++i )
        {
            parse_buttons( tokens[ i ], m_lights.size() );
        }
    }

    void parse_lights( const std::string& light_desc )
    {
        std::string target_lights = light_desc.substr( 1, light_desc.length() - 2 );
        m_lights.reserve( target_lights.length() );

        for( const char& c : target_lights )
        {
            switch( c )
            {
            case '.':
                m_lights.emplace_back( 0 );
                break;
            case '#':
                m_lights.emplace_back( 1 );
                break;
            default:
                break;
            }
        }
    }

    void parse_buttons(
        const std::string& button_desc,
        const size_t bit_depth
    )
    {
        // Fill a vector with the bits that are toggled by this button.
        std::vector<uint32_t> bits;
        fill_vector_from_string( bits, button_desc );

        // Convert the bit indices into a binary representation.
        std::vector<uint32_t> binary_desc( bit_depth, 0 );
        for( const uint32_t bit : bits )
        {
            binary_desc[ bit ] = 1;
        }

        // Push the binary representation onto the button vector.
        m_buttons.emplace_back( std::move( binary_desc ) );
    }

    void parse_joltage(
        const std::string& joltage_desc,
        const size_t bit_depth
    )
    {
        fill_vector_from_string( m_joltages, joltage_desc );
    }

    template<typename T>
    void fill_vector_from_string( T& vec, const std::string& str )
    {
        std::string num;
        std::string trimmed = str.substr( 1, str.size() - 2 );
        std::istringstream stream( trimmed );
        while( getline( stream, num, ',' ) )
        {
            vec.emplace_back( std::stoul( num ) );
        }
    }

    std::vector<uint32_t> m_lights;

    std::vector<std::vector<uint32_t>> m_buttons;
    combinationanator<uint32_t> m_bcombinations;

    std::vector<int32_t> m_joltages;
};