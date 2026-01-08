#pragma once

template<typename T>
class combinationanator
{
    using combi_container = std::vector<std::vector<T>>;

public:
    combinationanator()
    {
    };

    void init( std::vector<T>& items )
    {
        build_all_combinations( items );
    }

    typename combi_container::iterator begin ( void )
    {
        return m_combinations.begin();
    }

    typename combi_container::iterator end( void )
    {
        return m_combinations.end();
    }

private:
    void build_all_combinations( const std::vector<T>& v)
    {
        // Build all combinations of k elements of n
        size_t n = v.size();
        size_t k = 0;
        while( k <= n )
        {
            std::vector<T> c;
            build_combinations( c, v, 0, k++, n );
        }
    }

    void build_combinations(
        std::vector<T> c,
        const std::vector<T>& v,
        size_t i,
        size_t k,
        size_t n
    )
    {
        if( c.size() == k )
        {
            m_combinations.emplace_back( std::move( c ) );
        }
        else if( i < n )
        {
            c.emplace_back( v[ i ] );
            build_combinations( c, v, i + 1, k, n );

            c.pop_back();
            build_combinations( c, v, i + 1, k, n );
        }
    }

    combi_container m_combinations;
};