#pragma once
#include <map>
#include <unordered_map>

class circuit
{
public:
    circuit() = delete;
    circuit( size_t n1, size_t n2 )
    {
        insert_jbox( n1 );
        insert_jbox( n2 );
    }

    void insert_jbox( size_t n )
    {
        m_circuit.insert( std::make_pair( n, true ) );
    }

    void merge( circuit& rhs )
    {
        m_circuit.merge( rhs.m_circuit );
    }

    const size_t get_size( void ) const
    {
        return m_circuit.size();
    }

    class circuit_has_jbox
    {
    public:
        circuit_has_jbox() = delete;
        circuit_has_jbox( const size_t box_idx )
            : m_box( box_idx )
        {
        }

        bool operator()( const std::pair<size_t, circuit>& m ) const
        {
            const circuit& c = m.second;
            size_t count = c.m_circuit.count( m_box );

            return ( count != 0 );
        }

    private:
        size_t m_box;
    };

private:
    std::unordered_map<size_t, bool> m_circuit;
};

using circuit_map = std::multimap<size_t, circuit, std::greater<size_t>>;