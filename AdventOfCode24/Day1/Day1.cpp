// Day1_Part1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <assert.h>
#include <iostream>
#include <list>

#ifdef _DEBUG
void validate_lists( std::list<int32_t>& list1, std::list<int32_t>& list2 )
{
    assert( list1.size() == list2.size() );

    std::list<int32_t>::iterator iter = list1.begin();
    int32_t prev = *iter;
    iter++;

    while( iter != list1.end() )
    {
        assert( (*iter) >= prev );
        prev = *iter;
        iter++;
    }

    iter = list2.begin();
    prev = *iter;
    iter++;

    while( iter != list2.end() )
    {
        assert( (*iter) >= prev );
        prev = *iter;
        iter++;
    }
}
#endif

static bool read_input( std::list<int32_t>& list1, std::list<int32_t>& list2 )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is named "LocationIDs.csv" and
    // is present in the same folder as the executable.
    FILE* file = nullptr;
    errno_t err = fopen_s( &file, ".\\LocationIDs.csv", "r" );
    if( !file )
    {
        std::cout << "Failed to open input file." << std::endl;
        return false;
    }

    // Step 2: read the data.
    // Integers in the file represent "location IDs".
    // Each line contains two IDs separated by a comma.
    // The first entry is from list 1, the second from list 2.
    // Build each list by reading the lines from the input file
    // and placing the first value into list1 and the second
    // into list 2.
    int32_t first, second;
    while( 2 == fscanf_s( file, "%u,%u", &first, &second ) )
    {
        list1.emplace_back( first );
        list2.emplace_back( second );
    }

    // Step 3: close the input file and return success.
    fclose( file );
    return true;
}

static size_t calculate_distance( std::list<int32_t>& list1_sorted, std::list<int32_t>& list2_sorted )
{
    // Compare the "distance" between each entry
    // of the sorted lists.
    // Assumes both lists:
    // 1) have the same number of entries, and
    // 2) are sorted smallest to largest.
#ifdef _DEBUG
    validate_lists( list1_sorted, list2_sorted );
#endif

    size_t distance = 0;

    std::list<int32_t>::iterator iter1 = list1_sorted.begin();
    std::list<int32_t>::iterator iter2 = list2_sorted.begin();

    while( list1_sorted.end() != iter1 )
    {
        distance += std::abs( (*iter1) - (*iter2) );
        iter1++;
        iter2++;
    }

    return distance;
}

static size_t calculate_similarity( std::list<int32_t>& list1, std::list<int32_t>& list2 )
{
    // Compare the "similarity" between the lists.
    // Assumes both lists:
    // 1) are sorted smallest to largest.
    bool done = false;
    size_t similarity = 0;

    std::list<int32_t>::iterator iter1 = list1.begin();
    std::list<int32_t>::iterator iter2 = list2.begin();
    while( !done )
    {
        while( (*iter1) != (*iter2) )
        {
            if( (*iter1) < (*iter2) )
            {
                ++iter1;
            }
            else if( (*iter1) > (*iter2) )
            {
                ++iter2;
            }

            if( ( list1.end() == iter1 ) || ( list2.end() == iter2 ) )
            {
                done = true;
                break;
            }
        }

        if( !done )
        {
            int32_t val = *iter1;
            assert( val == *iter2 );

            size_t count1 = 0;
            while( val == *iter1 )
            {
                ++count1;
                ++iter1;
                if( list1.end() == iter1 )
                {
                    done = true;
                    break;
                }
            }

            size_t count2 = 0;
            while( val == *iter2 )
            {
                ++count2;
                ++iter2;

                if( list2.end() == iter2 )
                {
                    done = true;
                    break;
                }
            }

            similarity += val * count1 * count2;
        }
    }

    return similarity;
}

int main()
{
    // Step 1: Read the input CSV
    std::list<int32_t> list1;
    std::list<int32_t> list2;

    if( !read_input( list1, list2 ) )
    {
        return -1;
    }

    // Ensure both lists have the same number of entries before proceeding.
    if( list1.size() != list2.size() )
    {
        std::cout << "Lists have different sizes, must exit." << std::endl;
        return -2;
    }

    // Both parts benefit from sorted lists
    list1.sort();
    list2.sort();

    // Solve part 1 - distance calculation
    size_t distance = calculate_distance( list1, list2 );
    std::cout << "Distance = " << distance << "\n";

    // Solve part 2 - similarity score
    size_t similarity = calculate_similarity( list1, list2 );
    std::cout << "Similarity = " << similarity << std::endl;
}
