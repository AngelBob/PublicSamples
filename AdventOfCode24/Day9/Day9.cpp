// Day9.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class map_node_t
{
public:
    map_node_t( const size_t id, const bool is_f ) :
        file_id( id ),
        is_file( is_f )
    {
    }

    size_t get_file_id( void ) const
    {
        return file_id;
    }

    bool get_is_file( void ) const
    {
        return is_file;
    }

    void clear_is_file( void )
    {
        file_id = static_cast<size_t>( -1 );
        is_file = false;
    }

private:
    size_t file_id;
    bool   is_file;
};

static bool read_input(
    std::string& condensed_disk_map
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file( ".\\DiskMap_full.txt" );

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
    size_t id = 0;
    for( const char c : condensed_disk_map )
    {
        size_t count = static_cast<size_t>( c ) - '0';
        disk_map.insert( disk_map.end(), count, map_node_t( ( is_file ? id++ : static_cast< size_t >( -1 ) ), is_file ) );

        is_file = !is_file;
    }
}

static void move_files(
    std::vector<map_node_t>& disk_map
)
{
    // Move files from the end to the empty disk slots at the beginning.
    size_t empty_idx = 0;
    size_t file_idx = disk_map.size() - 1;

    while( 1 )
    {
        // Find next empty slot
        while( disk_map[ empty_idx ].get_is_file() &&
               empty_idx < disk_map.size() )
        {
            ++empty_idx;
        }

        // Find next file to move
        while( !disk_map[ file_idx ].get_is_file() &&
               0 <= file_idx )
        {
            --file_idx;
        }

        if( file_idx <= empty_idx )
        {
            // Processed all of the files at the end of the disk.
            break;
        }

        // Have an empty space and a file, move the file into the empty space
        // and replace the file with empty space
        disk_map[ empty_idx ] = disk_map[ file_idx ];
        disk_map[ file_idx ].clear_is_file();
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
