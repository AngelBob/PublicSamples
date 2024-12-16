// Day15.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <tuple>
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

    object( uint8_t start_row, uint8_t start_col, bool is_movable, bool is_robot, bool is_wide ) :
        row( start_row ),
        col( start_col ),
        wide_col( is_wide && !is_robot ? start_col + 1 : start_col ),
        movable( is_movable ),
        robot( is_robot ),
        wide( is_wide && !is_robot )
    {}

    bool can_move(
        std::vector<std::vector<std::shared_ptr<object>>>& grid,
        const enum WALK_DIR dir
    )
    {
        bool can_move = is_movable();
        if( can_move )
        {
            std::tuple<uint8_t, uint8_t, uint8_t> move = get_new_pos( dir );

            object* l_side = grid[ std::get<0>( move ) ][ std::get<1>( move ) ].get();
            object* r_side = grid[ std::get<0>( move ) ][ std::get<2>( move ) ].get();

            // Check if the left-hand side can move.
            if( ( nullptr != l_side ) && ( this != l_side ) )
            {
                can_move = l_side->can_move( grid, dir );
            }

            // Check if the right-hand side can move.
            if( is_wide() && can_move )
            {
                if( ( nullptr != r_side ) && ( this != r_side ) )
                {
                    can_move = r_side->can_move( grid, dir );
                }
            }
        }

        return can_move;
    }

    void do_move(
        std::vector<std::vector<std::shared_ptr<object>>>& grid,
        const enum WALK_DIR dir )
    {
        // Recursively move the objects.
        std::tuple<uint8_t, uint8_t, uint8_t> move = get_new_pos( dir );

        // Move the objects that are in the way.
        object* l_side = grid[ std::get<0>( move ) ][ std::get<1>( move ) ].get();
        if( ( nullptr != l_side ) && ( this != l_side ) )
        {
            l_side->do_move( grid, dir );
        }

        if( is_wide() )
        {
            object* r_side = grid[ std::get<0>( move ) ][ std::get<2>( move ) ].get();
            if( ( nullptr != r_side ) && ( this != r_side ) )
            {
                r_side->do_move( grid, dir );
            }
        }

        // Move this object
        if( WALK_DIR::WALK_EAST == dir )
        {
            // Must move the right hand side first.
            if( wide )
            {
                grid[ std::get<0>( move ) ][ std::get<2>( move ) ] = std::move( grid[ row ][ wide_col ] );
            }
            grid[ std::get<0>( move ) ][ std::get<1>( move ) ] = std::move( grid[ row ][ col ] );
        }
        else
        {
            // All other cases can/must move the left hand side first.
            grid[ std::get<0>( move ) ][ std::get<1>( move ) ] = std::move( grid[ row ][ col ] );
            if( wide )
            {
                grid[ std::get<0>( move ) ][ std::get<2>( move ) ] = std::move( grid[ row ][ wide_col ] );
            }
        }

        set_pos( move );
    }

    void set_pos( const std::tuple<uint8_t, uint8_t, uint8_t>& new_pos )
    {
        row = std::get<0>( new_pos );
        col = std::get<1>( new_pos );
        wide_col = std::get<2>( new_pos );
    }

    const std::tuple<uint8_t, uint8_t, uint8_t> get_pos( void ) const
    {
        return std::make_tuple( row, col, wide_col );
    }

    const bool is_movable( void ) const
    {
        return movable;
    }

    const bool is_robot( void ) const
    {
        return robot;
    }

    const bool is_wide( void ) const
    {
        return wide;
    }

    const std::tuple<uint8_t, uint8_t, uint8_t> get_new_pos( enum WALK_DIR dir ) const
    {
        uint8_t new_row = row + walk_data[ dir ].y_inc;
        uint8_t new_col = col + walk_data[ dir ].x_inc;
        uint8_t new_wide_col = new_col + ( wide ? 1 : 0 );

        return std::make_tuple( new_row, new_col, new_wide_col );
    }

    const int32_t get_gps_coordinates(
        std::vector<std::vector<std::shared_ptr<object>>>& grid,
        const uint8_t check_col ) const
    {
        int32_t gps_coord = 0;
        if( movable &&
            !robot &&
            ( !wide || check_col == col )
          )
        {
            // Only the left hand side reports GPS coordinates
            gps_coord = ( 100 * row ) + col;
        }

        return gps_coord;
    }

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

private:
    int8_t row;
    int8_t col, wide_col;

    const bool movable;
    const bool robot;
    const bool wide;
};

static bool read_input(
    std::vector<std::vector<std::shared_ptr<object>>>& grid,
    std::pair<uint8_t, uint8_t>& start_location,
    std::list<object::WALK_DIR>& movements,
    const bool do_resize
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
        std::vector<std::shared_ptr<object>> row_data( line.length() * ( do_resize ? 2 : 1 ) );
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

                std::shared_ptr<object> obj = std::make_shared<object>( row, col, is_movable, is_robot, do_resize );
                grid[ row ][ col ] = obj;

                if( !is_robot && do_resize )
                {
                    // Everything except the robot is now twice as wide
                    grid[ row ][ col + 1 ] = obj;
                }
            }
            col += do_resize ? 2 : 1;
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

static void dump_grid(
    std::vector<std::vector<std::shared_ptr<object>>>& grid,
    bool is_wide
)
{
#ifdef _DEBUG
    int32_t row_idx = 0;
    for( const auto& row : grid )
    {
        int32_t col_idx = 0;
        for( const auto& object : row )
        {
            char out( 'O' );
            if( object )
            {
                // Movable object
                if( object->is_movable() )
                {
                    if( object->is_robot() )
                    {
                        // Robot gets a different annotation.
                        out = '@';
                    }
                    else if( object->is_wide() )
                    {
                        // Wide objects get left and right side annotation.
                        std::tuple<uint8_t, uint8_t, uint8_t> pos = object->get_pos();
                        if( row_idx != std::get<0>( pos ) )
                        {
                            out = '*'; // Something is wrong.
                        }
                        else if( col_idx == std::get<1>( pos ) )
                        {
                            out = '['; // Left hand side
                        }
                        else if( col_idx == std::get<2>( pos ) )
                        {
                            out = ']'; // Right hand side
                        }
                        else
                        {
                            out = '-'; // Something is wrong.
                        }
                    }

                    std::cout << out;
                }
                else
                {
                    // Wall
                    std::cout << "#";
                }
            }
            else
            {
                // Empty space
                std::cout << ".";
            }
            ++col_idx;
        }
        std::cout << std::endl;
        ++row_idx;
    }
#endif
}

int main()
{
    std::vector<std::vector<std::shared_ptr<object>>> grid;
    std::pair<uint8_t, uint8_t> start_pos;
    std::list<object::WALK_DIR> movements;
    static constexpr bool do_resize = true;
    if( !read_input( grid, start_pos, movements, do_resize ) )
    {
        return -1;
    }

    size_t move = 1;
    object* robot = grid[ start_pos.first ][ start_pos.second ].get();
    for( const object::WALK_DIR dir : movements )
    {
        //dump_grid( grid, do_resize );
        if( robot->can_move( grid, dir ) )
        {
            robot->do_move( grid, dir );
        }
        ++move;
    }

    int32_t gps_sum = 0;
    for( const auto& row : grid )
    {
        uint8_t col = 0;
        for( const auto& object : row )
        {
            if( object )
            {
                gps_sum += object->get_gps_coordinates( grid, col );
            }
            ++col;
        }
    }
    std::cout << "The sum of the GPS coordinates is " << gps_sum << "\n";
}
