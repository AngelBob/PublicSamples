// Day5.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

static bool read_input(
    std::multimap<int32_t, int32_t>& rules,
    std::vector<std::vector<int32_t>>& pages
    )
{
    // Open the input files and read the data.
    static const std::array<std::pair<std::string, char>, 2> inputs{{
        { ".\\OrderingRules.txt", '|' },
        { ".\\PageUpdates.txt", ',' }
    }};

    for( auto& input : inputs )
    {
        // Step 1: open the input file.
        std::ifstream file( input.first );

        // Step 2: read each line and split on the separator character.
        std::string line;
        while( std::getline( file, line ) )
        {
            std::vector<int32_t> values;

            std::stringstream ss( line );
            std::string val;
            while( std::getline( ss, val, input.second ) )
            {
                int32_t number = atoi( val.c_str() );
                values.emplace_back( number );
            }

            if( '|' == input.second )
            {
                rules.insert( std::make_pair( values[ 0 ], values[ 1 ] ) );
            }
            else
            {
                pages.emplace_back( values );
            }
        }
    }

    // Step 3: return success or failure.
    return ( rules.size() && pages.size() );
}

static void check_page_order(
    const std::multimap<int32_t, int32_t>& rules,
    const std::vector<std::vector<int32_t>>& pages_list,
    std::vector<std::vector<int32_t>>& ordered_pages
)
{
    for( auto& pages : pages_list )
    {
        std::multimap<int32_t, int32_t>::const_iterator rule_iter = rules.begin();
        while( rules.end() != rule_iter )
        {
            std::vector<int32_t>::const_iterator first_page =
                std::find( pages.begin(), pages.end(), (*rule_iter).first );
            std::vector<int32_t>::const_iterator second_page =
                std::find( pages.begin(), pages.end(), (*rule_iter).second );
            if( pages.end() != first_page &&
                pages.end() != second_page &&
                first_page >= second_page )
            {
                break;
            }
            ++rule_iter;
        }

        if( rules.end() == rule_iter )
        {
            // All rules matched
            ordered_pages.emplace_back( pages );
        }
    }
}

static size_t sum_middle_pages( const std::vector<std::vector<int32_t>>& ordered_pages )
{
    size_t sum = 0;

    for( auto& pages : ordered_pages )
    {
        size_t middle_index = pages.size() / 2;
        sum += pages[ middle_index ];
    }

    return sum;
}

int main()
{
    std::multimap<int32_t, int32_t> rules;
    std::vector<std::vector<int32_t>> pages;

    if( !read_input( rules, pages ) )
    {
        return -1;
    }

    std::vector<std::vector<int32_t>> ordered_pages;
    check_page_order( rules, pages, ordered_pages );

    size_t total = sum_middle_pages( ordered_pages );
    std::cout << "Page sum = " << total << "\n";
}
