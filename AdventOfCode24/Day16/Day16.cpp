// Day16.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <climits>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename TupleT, std::size_t... Idx>
auto subtuple_impl( const TupleT& t, std::index_sequence<Idx...> )
{
    return std::make_tuple( std::get<Idx>( t )... );
}

template<std::size_t N, typename TupleT>
auto subtuple( const TupleT& t )
{
    return subtuple_impl( t, std::make_index_sequence<N>{} );
}

enum WALK_DIR
{
    WALK_START = 0,
    WALK_NORTH = 0,
    WALK_EAST,
    WALK_SOUTH,
    WALK_WEST,
    WALK_END
};

typedef struct walk_data
{
    enum WALK_DIR  dir;
    int8_t         x_inc;
    int8_t         y_inc;
} walk_data_t;

class state
{
public:
    state() = delete;

    state( const size_t x, const size_t y, const WALK_DIR dir ) :
        position( x, y ),
        dir( dir )
    {
    }

    state( std::tuple<size_t, size_t, const WALK_DIR> in )
    {
        std::tie( position.first, position.second, std::ignore ) = in;
        std::tie( std::ignore,    std::ignore,     dir ) = in;
    }

    std::array<std::pair<size_t, state>, 3> get_neighbors( void ) const
    {
        enum WALK_DIR dir_f = dir;
        const state s0( position.first  + walk_data[ dir_f ].x_inc,
                        position.second + walk_data[ dir_f ].y_inc,
                        dir_f );

        enum WALK_DIR dir_r = static_cast<WALK_DIR>( ( static_cast<int8_t>( dir ) + 1 ) % WALK_DIR::WALK_END );
        const state s1( position.first  + walk_data[ dir_r ].x_inc,
                        position.second + walk_data[ dir_r ].y_inc,
                        dir_r );

        enum WALK_DIR dir_l = static_cast<WALK_DIR>( ( static_cast<int8_t>( dir ) + WALK_DIR::WALK_END - 1 ) % WALK_DIR::WALK_END );
        const state s2( position.first  + walk_data[ dir_l ].x_inc,
                        position.second + walk_data[ dir_l ].y_inc,
                        dir_l );

        std::array<std::pair<size_t, state>, 3> target_tiles{{
            {    1, s0 },
            { 1001, s1 },
            { 1001, s2 }
        }};

        return target_tiles;
    }

    const std::pair<size_t, size_t>& get_position( void ) const
    {
        return position;
    }

    const enum WALK_DIR get_dir( void ) const
    {
        return dir;
    }

    bool operator<(const state& b) const
    {
        bool pos_less = ( position < b.get_position() );
        bool dir_less = ( position == b.get_position() ) && ( dir < b.get_dir() );

        return pos_less || dir_less;
    }

    bool operator==( const state& b ) const
    {
        const std::pair<size_t, size_t>& b_pos = b.get_position();
        enum WALK_DIR b_dir = b.get_dir();

        return ( position.first  == b_pos.first  ) &&
               ( position.second == b_pos.second ) &&
               ( dir == b_dir );
    }

    struct state_hash
    {
        std::size_t operator()( const state& s ) const
        {
            size_t state_hash = static_cast<size_t>( s.get_dir() );
            const std::pair<size_t, size_t>& pos = s.get_position();
            if constexpr ( sizeof( size_t ) == sizeof( uint64_t ) )
            {
                uint32_t pos_hash = static_cast<uint32_t>( pos.first );
                pos_hash <<= 16;
                pos_hash |= static_cast<uint32_t>( pos.second & 0xFFFF );

                state_hash |= ( static_cast<size_t>( pos_hash ) << 32);
            }
            else
            {
                uint16_t pos_hash = static_cast<uint16_t>( pos.first );
                pos_hash <<= 8;
                pos_hash |= static_cast<uint16_t>( pos.second & 0xFF );

                state_hash |= ( static_cast<size_t>( pos_hash ) << 16 );
            }

            return state_hash;
        }
    };

private:
    static constexpr std::array<walk_data_t, WALK_END> walk_data{{
        { WALK_DIR::WALK_NORTH,  0, -1},
        { WALK_DIR::WALK_EAST,   1,  0},
        { WALK_DIR::WALK_SOUTH,  0,  1},
        { WALK_DIR::WALK_WEST,  -1,  0},
    }};

    std::pair<size_t, size_t> position;
    enum WALK_DIR dir;
};

struct pair_hash
{
    inline std::size_t operator()( const std::pair<size_t, size_t>& v ) const
    {
        size_t pair_hash = 0;
        if constexpr ( sizeof( size_t ) == sizeof( uint64_t ) )
        {
            pair_hash = v.first;
            pair_hash <<= 32;
            pair_hash |= v.second & 0xFFFFFFFF;
        }
        else
        {
            pair_hash = v.first;
            pair_hash <<= 16;
            pair_hash |= v.second & 0xFFFF;
        }

        return pair_hash;
    }
};

static bool read_input(
    std::vector<std::string>& map,
    std::tuple<size_t, size_t, enum WALK_DIR>& start_pos,
    std::pair<size_t, size_t>& end_pos
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map_full.txt" );

    // Step 2: read each line and insert individual characters into the grid.
    size_t row = 0;
    std::string map_row;
    while( std::getline( file, map_row ) )
    {
        map.emplace_back( map_row );

        // Deer always starts by walking east.
        size_t start_x = map_row.find( 'S' );
        if( map_row.npos != start_x )
        {
            std::get<0>( start_pos ) = start_x;
            std::get<1>( start_pos ) = row;
            std::get<2>( start_pos ) = WALK_DIR::WALK_EAST;
        }

        size_t end_x = map_row.find( 'E' );
        if( map_row.npos != end_x )
        {
            end_pos = std::make_pair( end_x, row );
        }

        ++row;
    }

    // Step 3: return success or failure.
    return true;
}

static void do_backtrack(
    std::unordered_map<
        state,
        std::pair<size_t, std::unordered_set<state, state::state_hash>>,
        state::state_hash>& visited,
    const std::pair<size_t, size_t> end
)
{
    std::unordered_set<state, state::state_hash> backtracked;
    std::unordered_set<std::pair<size_t, size_t>, pair_hash> positions;
    std::deque<state> backtrack{
        { end.first, end.second, WALK_NORTH },
        { end.first, end.second, WALK_EAST },
        { end.first, end.second, WALK_SOUTH },
        { end.first, end.second, WALK_WEST }
    };

    while( !backtrack.empty() )
    {
        state b_state = backtrack[ 0 ];
        backtrack.pop_front();

        positions.insert( b_state.get_position() );

        backtracked.insert( b_state );

        auto it = visited.find( b_state );
        if( visited.end() == it )
        {
            continue;
        }

        auto& prev_states = (*it).second.second;
        for( const state& prev_state : prev_states )
        {
            if( !backtracked.contains( prev_state ) )
            {
                backtrack.push_back( prev_state );
            }
        }
    }

    std::cout << "The number of prime seats is " <<
                 std::to_string( positions.size() ) << std::endl;
}
static void solve_maze(
    const std::vector<std::string>& map,
    const state& start,
    std::pair<size_t, size_t> end
)
{
    std::priority_queue<std::pair<size_t, state>,
                        std::vector<std::pair<size_t, state>>,
                        std::greater<std::pair<size_t, state>>> pq;
    std::unordered_map<state,
                       std::pair<size_t, std::unordered_set<state, state::state_hash>>,
                       state::state_hash> visited;

    size_t end_score = std::numeric_limits<size_t>::max();

    pq.push( { 0, start } ); // ( distance, node )
    while( !pq.empty() )
    {
        auto [ score, state ] = pq.top();
        pq.pop();

        if( end == state.get_position() )
        {
            // Found the end, update the final score.
            end_score = score;
            break;
        }

        for( auto& [ cost, n_state ] : state.get_neighbors() )
        {
            const std::pair<size_t, size_t>& pos_n = n_state.get_position();
            if( '#' == map[ pos_n.second ][ pos_n.first ] )
            {
                // Not a viable move, so skip it.
                continue;
            }

            size_t neighbor_score = score + cost;
            if( end_score < neighbor_score )
            {
                // Already a more expensive path to the end, so skip it.
                continue;
            }

            auto it = visited.find( n_state );
            if( visited.end() != it )
            {
                auto& [ prev_score, prev_states ] = (*it).second;
                if( prev_score < neighbor_score )
                {
                    // Already a more expensive path to this tile, skip it.
                    continue;
                }
                else if( prev_score == neighbor_score )
                {
                    // Another path to this tile with a low score.
                    prev_states.insert( state );
                }
            }
            else
            {
                // Update the low score for this tile.
                visited[ n_state ] = { neighbor_score, { state } };
            }

            pq.push( { neighbor_score, std::move( n_state ) } );
        }
    }

    std::cout << "The lowest path score is " <<
        std::to_string( end_score ) << std::endl;

    // Found the end, now walk backwards to find all of the locations with the
    // same low score.
    do_backtrack( visited, end );
}

int main()
{
    std::vector<std::string> map;
    std::tuple<size_t, size_t, enum WALK_DIR> start_pos;
    std::pair<size_t, size_t> end_pos;
    if( !read_input( map, start_pos, end_pos ) )
    {
        return -1;
    }

    const state in_state( start_pos );
    solve_maze( map, in_state, end_pos );
}