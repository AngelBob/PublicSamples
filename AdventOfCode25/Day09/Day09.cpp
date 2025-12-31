// Day09.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <string>

static bool read_input( const std::string& filename )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is present in a sub-folder.
    std::ifstream file( filename );

    // Step 2: read the lines and create the input data.
    while( !file.eof() )
    {
        std::string line;
        std::getline( file, line );
    }

    // Step 3: return success or failure.
    return true;
}

int main()
{
#if 1
    static const std::string filename( ".\\Data\\Input_test.txt" );
#else
    static const std::string filename( ".\\Data\\Input.txt" );
#endif

    if( !read_input( filename ) )
    {
        return -1;
    }
}