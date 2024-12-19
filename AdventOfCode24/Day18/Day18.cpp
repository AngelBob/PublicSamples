// Day18.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

static constexpr std::array<walk_data_t, WALK_END> walk_data{ {
    { WALK_DIR::WALK_NORTH,  0, -1},
    { WALK_DIR::WALK_EAST,   1,  0},
    { WALK_DIR::WALK_SOUTH,  0,  1},
    { WALK_DIR::WALK_WEST,  -1,  0},
} };

static bool read_input(
    std::vector<std::pair<int32_t, int32_t>>& simulation_coords
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2: read each line and insert individual characters into the map.
    static const std::regex coords( "([0-9]+),([0-9]+)" );
    size_t depth = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        // Do a regex match to extract the calibration data from the line.
        std::smatch reg_match;
        if( std::regex_match( line.cbegin(), line.cend(), reg_match, coords ) )
        {
            std::pair<uint32_t, uint32_t> line_data;
            simulation_coords.emplace_back(
                std::make_pair( std::stol( reg_match[ 1 ] ), std::stol( reg_match[ 2 ] ) )
            );
        }

        ++depth;
    }

    // Step 3: return success or failure.
    return true;
}

static void add_blockage(
    std::vector<std::vector<char>>& map,
    const std::vector<std::pair<int32_t, int32_t>>& simulation_coords,
    const int32_t coords_index
)
{
    map[ simulation_coords[ coords_index ].second ][ simulation_coords[ coords_index ].first ] = '#';
}

static void make_map(
    const std::vector<std::pair<int32_t, int32_t>>& simulation_coords,
    const int32_t simulation_depth,
    const int32_t map_dimension,
    std::vector<std::vector<char>>& map
)
{
    // Convert the coordinates into a map layout
    static const char blockage = '#';
    static const char free = '.';

    // Fill the map with free space
    std::vector<char> row;
    row.insert( row.begin(), map_dimension, free );
    map.insert( map.begin(), map_dimension, row );
    row.clear();

    // Walk the simulation coordinates inserting blockages
    for( int32_t idx = 0; idx < simulation_depth; ++idx )
    {
        add_blockage( map, simulation_coords, idx );
    }

    // Check the map has an exit point
    if( '#' == map[ map_dimension - 1 ][ map_dimension - 1 ] )
    {
        __debugbreak(); // Map is not solvable
    }
}

// Hash function for std::pair<int32_t, int32_t>
struct PointHash
{
    size_t operator()( const std::pair<int32_t, int32_t>& p ) const
    {
        return std::hash<int32_t>()( p.first ) ^ ( std::hash<int32_t>()( p.second ) << 1 );
    }
};

// Node structure for priority queue
class Node
{
public:
    Node() = delete; // No default constructor for this class

    Node( std::pair<int32_t, int32_t> point, int32_t g, int32_t h ) :
        point( point ),
        g( g ),
        h( h )
    {
    }

    const int32_t f( void ) const
    {
        return g + h;
    }

    const bool operator>( const Node& other ) const
    {
        return f() > other.f();
    }

    std::pair<int32_t, int32_t> get_point( void ) const
    {
        return point;
    }

    int32_t get_g( void ) const
    {
        return g;
    }

private:
    std::pair<int32_t, int32_t> point; /* x, y which is opposite of row, col */

    int32_t g;
    int32_t h;
};

// Manhattan distance heuristic
static int32_t manhattan(
    const std::pair<int32_t, int32_t>& a,
    const std::pair<int32_t, int32_t>& b )
{
    return std::abs( a.first - b.first ) + std::abs( a.second - b.second );
}

// A* pathfinding algorithm
static std::vector<std::pair<int32_t, int32_t>> a_star(
    const std::vector<std::vector<char>>& map,
    const std::pair<int32_t, int32_t>& start,
    const std::pair<int32_t, int32_t>& end
)
{
    size_t rows = map.size();
    size_t cols = map[ 0 ].size();

    // Priority queue for open set
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
    openSet.emplace( start, 0, manhattan( start, end ) );

    // Map to track visited nodes and parent pointers
    std::unordered_set<std::pair<int32_t, int32_t>, PointHash> closedSet;
    std::unordered_map<std::pair<int32_t, int32_t>, std::pair<int32_t, int32_t>, PointHash> cameFrom;

    // Map to track the g-values (costs)
    std::unordered_map<std::pair<int32_t, int32_t>, int32_t, PointHash> gScore;
    gScore[ start ] = 0;

    while( !openSet.empty() )
    {
        Node current = openSet.top();
        openSet.pop();

        // If the goal is reached, reconstruct the path
        if( current.get_point() == end )
        {
            std::vector<std::pair<int32_t, int32_t>> path;
            std::pair<int32_t, int32_t> p = end;
            while( cameFrom.find( p ) != cameFrom.end() )
            {
                path.push_back( p );
                p = cameFrom[ p ];
            }
            path.push_back( start );
            reverse( path.begin(), path.end() );
            return path;
        }

        // Mark current node as visited
        closedSet.insert( current.get_point() );

        // Explore neighbors
        for( const auto& dir : walk_data )
        {
            std::pair<int32_t, int32_t> neighbor(
                current.get_point().first + dir.x_inc,
                current.get_point().second + dir.y_inc
            );

            // Check if neighbor is within bounds and not an obstacle
            if( neighbor.second < 0 || rows <= neighbor.second ||
                neighbor.first < 0  || cols <= neighbor.first  ||
                map[ neighbor.second ][ neighbor.first ] == '#' )
            {
                continue;
            }

            // Skip visited nodes
            if( closedSet.find( neighbor ) != closedSet.end() )
            {
                continue;
            }

            // Calculate tentative g-score
            int32_t tentative_gScore = current.get_g() + 1;

            // If this path to neighbor is better, update its scores and enqueue
            if( gScore.find( neighbor ) == gScore.end() ||
                tentative_gScore < gScore[ neighbor ] )
            {
                cameFrom[ neighbor ] = current.get_point();
                gScore[ neighbor ] = tentative_gScore;
                openSet.emplace( neighbor, tentative_gScore, manhattan( neighbor, end ) );
            }
        }
    }

    // Return empty path if no solution found
    return {};
}

int32_t main()
{
    static const int32_t simulation_depth = 12;
    static const int32_t map_dimension = 7;
    static const bool find_blockage = true;

    std::vector<std::pair<int32_t, int32_t>> simulation_coordinates;
    if( !read_input( simulation_coordinates ) )
    {
        return -1;
    }

    std::vector<std::vector<char>> map;
    make_map( simulation_coordinates, simulation_depth, map_dimension, map );

    int32_t next_drop = simulation_depth;
    while( 1 )
    {
        if( find_blockage )
        {
            add_blockage( map, simulation_coordinates, next_drop++ );
        }
        else
        {
#ifdef _DEBUG
            // Dump the starting map
            for( auto& row : map )
            {
                for( auto& cell : row )
                {
                    std::cout << cell;
                }
                std::cout << std::endl;
            }
#endif
        }

        std::vector<std::pair<int32_t, int32_t>> path = a_star(
            map,
            { 0, 0 },
            { map_dimension - 1, map_dimension - 1 }
        );

        if( !path.empty() )
        {
            std::cout << "Path found:\n";
            std::cout << "Minimum steps = " << path.size() - 1 << "\n";

            if( !find_blockage )
            {
#ifdef _DEBUG
            // Dump the path
                for( const auto& p : path )
                {
                    std::cout << "(" << p.second << ", " << p.first << ") ";
                }
                std::cout << std::endl;

                break;
#endif
            }
        }
        else
        {
            std::cout << "No path found.\n";
            std::cout << "First blocker: " <<
                         std::to_string( simulation_coordinates[ next_drop - 1 ].first ) <<
                         ","
                         << simulation_coordinates[ next_drop - 1 ].second << "\n";
            break;
        }
    }
}