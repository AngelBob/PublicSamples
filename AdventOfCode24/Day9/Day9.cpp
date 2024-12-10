// Day9.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class map_node_t
{
public:
    map_node_t( const int32_t id, const int32_t cont_size, const bool is_f ) :
        file_id( id ),
        contiguous_size( cont_size ),
        is_file( is_f )
    {
    }

    int32_t get_file_id( void ) const
    {
        return file_id;
    }

    int32_t get_contiguous_size( void ) const
    {
        return contiguous_size;
    }

    void set_contiguous_size( int32_t new_size )
    {
        contiguous_size = new_size;
    }

    bool get_is_file( void ) const
    {
        return is_file;
    }

    void clear_is_file( void )
    {
        file_id = -1;
        is_file = false;
    }

private:
    int32_t file_id;
    int32_t contiguous_size;
    bool   is_file;
};

static bool read_input(
    std::string& condensed_disk_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\DiskMap.txt" );

    // Step 2: read the single line of input.
    std::getline( file, condensed_disk_map );

    // Step 3: return success or failure.
    return true;
}

static void expand_disk_map(
    const std::string& condensed_disk_map,
    std::vector<map_node_t>& disk_map
)
{
    bool is_file = true;
    int32_t id = 0;
    for( const char c : condensed_disk_map )
    {
        int32_t count = static_cast< int32_t >( c ) - '0';
        disk_map.insert(
            disk_map.end(),
            count,
            map_node_t( is_file ? id++ : -1, count, is_file ) );

        is_file = !is_file;
    }
}

static void move_files(
    std::vector<map_node_t>& disk_map
)
{
    // Move files from the end to the empty disk slots at the beginning.
    int32_t file_idx = static_cast<int32_t>( disk_map.size() ) - 1;

    // Try moving all of the files into left most space into which they fit.
    while( 0 <= file_idx )
    {
        // Find the next file to move
        while( !disk_map[ file_idx ].get_is_file() &&
               0 <= file_idx )
        {
            file_idx -= disk_map[ file_idx ].get_contiguous_size();
        }

        int32_t file_size = disk_map[ file_idx ].get_contiguous_size();
        int32_t empty_idx = 0;
        while( 0 <= file_idx )
        {
            while( disk_map[ empty_idx ].get_is_file() &&
                   empty_idx < disk_map.size() )
            {
                empty_idx += disk_map[ empty_idx ].get_contiguous_size();
            }

            int32_t empty_size = disk_map[ empty_idx ].get_contiguous_size();
            if( file_idx <= empty_idx )
            {
                // This file can't move left, skip it and continue.
                file_idx -= disk_map[ file_idx ].get_contiguous_size();
                break;
            }
            else if( file_size <= empty_size )
            {
                // Have an empty space big enough to hold the entire file.
                // Move the file into the empty space and replace the file with
                // empty space.
                for( int32_t move_idx = 0; move_idx < file_size; ++move_idx )
                {
                    disk_map[ empty_idx++ ] = disk_map[ file_idx ];
                    disk_map[ file_idx-- ].clear_is_file();
                }

                // Update the size of the empty space
                int32_t remainder = empty_size - file_size;
                for( int32_t clear_idx = 0; clear_idx < remainder; ++clear_idx )
                {
                    disk_map[ empty_idx ].set_contiguous_size( remainder );
                }

                // Move to the next file.
                break;
            }
            else
            {
                // Skip this empty space, it wasn't big enough.
                empty_idx += disk_map[ empty_idx ].get_contiguous_size();
            }
        }
    }
}

static size_t calculate_disk_checksum(
    const std::vector<map_node_t>& disk_map
)
{
    size_t checksum_value = 0;
    for( size_t checksum_idx = 0; checksum_idx < disk_map.size(); ++checksum_idx )
    {
        if( disk_map[ checksum_idx ].get_is_file() )
        {
            checksum_value += ( disk_map[ checksum_idx ].get_file_id() * checksum_idx );
        }
    }

    return checksum_value;
}

int main()
{
    std::string condensed_disk_map;
    if( !read_input( condensed_disk_map ) )
    {
        return -1;
    }

    std::vector<map_node_t> disk_map;
    expand_disk_map( condensed_disk_map, disk_map );

    move_files( disk_map );

    size_t checksum = calculate_disk_checksum( disk_map );
    std::cout << "The checksum is " << checksum << "\n";
}
