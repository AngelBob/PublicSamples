// Day15.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

class object
{
public:
    enum WALK_DIR
    {
        WALK_START = 0,
        WALK_NORTH = 0,
        WALK_EAST,
        WALK_SOUTH,
        WALK_WEST,
        WALK_END
    };

    object() = delete; // No default constructor

    object( uint8_t start_row, uint8_t start_col, bool movable, bool robot ) :
        row( start_row ),
        col( start_col ),
        is_movable( movable ),
        is_robot( robot )
    {
    }

    void do_move(
        std::vector<std::vector<std::unique_ptr<object>>>& grid,
        const enum WALK_DIR dir )
    {
        if( is_movable )
        {
            uint8_t new_row = row + walk_data[ dir ].y_inc;
            uint8_t new_col = col + walk_data[ dir ].x_inc;

            // If the destination is not empty, try moving that object first.
            if( nullptr != grid[ new_row ][ new_col ].get() )
            {
                grid[ new_row ][ new_col ]->do_move( grid, dir );
            }

            // If the destination is empty, moving this object.
            if( nullptr == grid[ new_row ][ new_col ].get() )
            {
                grid[ new_row ][ new_col ] = std::move( grid[ row ][ col ] );
                /*grid[x][y] = nullptr;*/
                row = new_row;
                col = new_col;
            }
        }
    }

    const int32_t get_gps_coordinates( void ) const
    {
        int32_t gps_coord = 0;
        if( is_movable && !is_robot )
        {
            gps_coord = ( 100 * row ) + col;
        }

        return gps_coord;
    }

private:
    typedef struct walk_data
    {
        enum WALK_DIR  dir;
        int8_t         x_inc;
        int8_t         y_inc;
    } walk_data_t;

    static constexpr std::array<walk_data_t, WALK_END> walk_data{{
        { WALK_DIR::WALK_NORTH,  0, -1},
        { WALK_DIR::WALK_EAST,   1,  0},
        { WALK_DIR::WALK_SOUTH,  0,  1},
        { WALK_DIR::WALK_WEST,  -1,  0},
    }};

    int8_t row;
    int8_t col;

    bool is_movable;
    bool is_robot;
};

static bool read_input(
    std::vector<std::vector<std::unique_ptr<object>>>& grid,
    std::pair<uint8_t, uint8_t>& start_location,
    std::list<object::WALK_DIR>& movements
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\Map.txt" );

    // Step 2a: read each line and insert objects into the grid.
    int8_t row = 0;
    std::string line;
    while( std::getline( file, line ) )
    {
        std::vector<std::unique_ptr<object>> row_data( line.length() );
        grid.emplace_back( std::move( row_data ) );

        int8_t col = 0;
        for( const char c : line )
        {
            if( '.' != c )
            {
                bool is_movable = ( 'O' == c ) || ( '@' == c );
                bool is_robot = false;
                if( '@' == c )
                {
                    start_location = std::make_pair( row, col );
                    is_robot = true;
                }

                grid[ row ][ col ] = std::move( std::make_unique<object>( row, col, is_movable, is_robot ) );
            }
            ++col;
        }
        ++row;
    }

    // Step 2b: read each movement line
    file.close();
    file.open( ".\\Moves.txt" );
    while( std::getline( file, line ) )
    {
        for( const char c : line )
        {
            switch( c )
            {
            case '^':
                movements.emplace_back( object::WALK_DIR::WALK_NORTH );
                break;
            case '>':
                movements.emplace_back( object::WALK_DIR::WALK_EAST );
                break;
            case 'V':
            case 'v':
                movements.emplace_back( object::WALK_DIR::WALK_SOUTH );
                break;
            case '<':
                movements.emplace_back( object::WALK_DIR::WALK_WEST );
                break;
            default:
                __debugbreak();
            }
        }
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
    std::vector<std::vector<std::unique_ptr<object>>> grid;
    std::pair<uint8_t, uint8_t> start_pos;
    std::list<object::WALK_DIR> movements;
    if( !read_input( grid, start_pos, movements ) )
    {
        return -1;
    }

    object* robot = grid[ start_pos.first ][ start_pos.second ].get();
    for( const object::WALK_DIR dir : movements )
    {
        robot->do_move( grid, dir );
    }

    int32_t gps_sum = 0;
    for( const auto& row : grid )
    {
        for( const auto& object : row )
        {
            if( object )
            {
                gps_sum += object->get_gps_coordinates();
            }
        }
    }
    std::cout << "The sum of the GPS coordinates is " << gps_sum << "\n";
}
