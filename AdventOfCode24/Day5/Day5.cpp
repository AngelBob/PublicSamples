// Day5.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <vector>

static bool read_input(
    std::multimap<int32_t, int32_t>& rules,
    std::vector<std::list<int32_t>>& pages
    )
{
    // Open the input files and read the data.
    static const std::array<std::pair<std::string, char>, 2> inputs{{
        { ".\\OrderingRules.txt", '|' },
        { ".\\PageUpdates.txt", ',' }
    }};

    std::multimap<int32_t, int32_t> sorted_rules;
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
                std::multimap<int32_t, int32_t>::iterator upper_b = sorted_rules.upper_bound( values[ 1 ] );
                std::multimap<int32_t, int32_t>::iterator lower_b = sorted_rules.lower_bound( values[ 1 ] );
                while( lower_b != upper_b )
                {
                    if( sorted_rules.end() != std::next(lower_b) &&
                        (*lower_b).first == values[ 1 ] &&
                        (*lower_b).second > values[ 0 ] )
                    {
                        break;
                    }
                    ++lower_b;
                }
                 sorted_rules.emplace_hint( lower_b, std::make_pair( values[ 1 ], values[ 0 ] ) );
            }
            else
            {
                pages.emplace_back( values.begin(), values.end() );
            }
        }
    }

    for( auto& rule : sorted_rules )
    {
        rules.insert( std::make_pair( rule.second, rule.first ) );
    }

    // Step 3: return success or failure.
    return ( rules.size() && pages.size() );
}

static void check_page_order(
    const std::multimap<int32_t, int32_t>& rules,
    std::vector<std::list<int32_t>>& unordered_pages,
    std::vector<std::list<int32_t>>& ordered_pages
)
{
    std::vector<std::list<int32_t>>::iterator pages_iter = unordered_pages.begin();
    while( unordered_pages.end() != pages_iter )
    {
        bool do_move = true;
        std::multimap<int32_t, int32_t>::const_iterator rule_iter = rules.begin();
        while( rules.end() != rule_iter )
        {
            std::list<int32_t>::iterator second_page =
                std::find( (*pages_iter).begin(), (*pages_iter).end(), (*rule_iter).second );
            if( (*pages_iter).end() != second_page)
            {
                std::list<int32_t>::iterator first_page =
                    std::find( second_page, (*pages_iter).end(), (*rule_iter).first );
                if( (*pages_iter).end() != first_page )
                {
                    // This rule is violated; move the first page in front of the second.
                    (*pages_iter).insert( second_page, ( *first_page ) );
                    (*pages_iter).erase( first_page );
                    do_move = false;
                }
            }
            ++rule_iter;
        }

        if( do_move )
        {
            // All rules matched
            ordered_pages.emplace_back( std::move( *pages_iter ) );
            pages_iter = unordered_pages.erase( pages_iter );
        }
        else
        {
            ++pages_iter;
        }
    }
}

static size_t sum_middle_pages( const std::vector<std::list<int32_t>>& ordered_pages )
{
    size_t sum = 0;

    for( auto& pages : ordered_pages )
    {
        size_t middle_index = pages.size() / 2;
        std::list<int32_t>::const_iterator value = std::next( pages.begin(), middle_index );

        sum += *value;
    }

    return sum;
}

int main()
{
    std::multimap<int32_t, int32_t> rules;
    std::vector<std::list<int32_t>> pages;

    if( !read_input( rules, pages ) )
    {
        return -1;
    }

    std::vector<std::list<int32_t>> ordered_pages;
    check_page_order( rules, pages, ordered_pages );

    size_t total = sum_middle_pages( ordered_pages );
    std::cout << "Page sum = " << total << "\n";

    total = sum_middle_pages( pages );
    std::cout << "Fixed page sum = " << total << std::endl;
}
