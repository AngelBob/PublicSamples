// Day24.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>

static bool read_input(
    std::map<std::string, bool>& registers,
    std::map<std::string, std::tuple<std::string, std::string, std::string>>& operations
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Logic.txt");

    // Step 2: read each line and insert operations into the map.
    bool have_registers = false;
    std::string line;
    while( std::getline( file, line ) )
    {
        if (0 == line.length())
        {
            have_registers = true;
            continue;
        }

        if( !have_registers )
        {
            std::string reg( line.substr( 0, line.find( ':' ) ) );
            bool val = std::stoi( line.substr( line.find( ':' ) + 2, line.length() - 1 ) ) ? true : false;

            registers.insert( std::make_pair( reg, val ) );
        }
        else
        {
            std::string first_input( line.substr( 0, 3 ) );
            std::string operation( line.substr( 4, line.find( ' ', 4 ) - 4 ) );
            std::string second_input( line.substr( line.find( ' ', 4 ) + 1, 3 ) );

            std::string output( line.substr( line.length() - 3, line.length() - 1 ) );

            if( 0 != operations.count( output ) )
            {
                // Assert any given output is only used once.
                __debugbreak();
            }
            operations.insert( std::make_pair( output, std::make_tuple( first_input, second_input, operation ) ) );
        }
    }

    // Step 3: return success or failure.
    return( 0 != registers.size() && 0 != operations.size() );
}

int main()
{
    std::map<std::string, bool> registers;
    std::map<std::string, std::tuple<std::string, std::string, std::string>> operations;
    if( !read_input( registers, operations ) )
    {
        return -1;
    }
}
