// Day2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <assert.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static bool read_input( std::vector<std::vector<int32_t>>& reports )
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    // Assumes the input file is named "Reports.txt" and
    // is present in the same folder as the executable.
    std::ifstream file( ".\\Reports.txt" );

    // Step 2: read the data.
    // Integers in the file represent levels.
    // Each line contains multiple levels separated by a space.
    std::string line;
    while( std::getline( file, line ) )
    {
        reports.emplace_back();
        std::istringstream ss( line );
        while( !ss.eof() )
        {
            int32_t val;
            ss >> val;
            reports.back().emplace_back( val );
        }
    }

    // Step 3: return success or failure.
    return ( 0 != reports.size() );
}

static bool is_report_safe( const std::vector<int32_t>& levels )
{
    size_t level_cnt = levels.size();
    assert( level_cnt > 1 );

    bool start_direction = levels[ 0 ] < levels[ 1 ];
    bool is_safe = true;

    for( size_t index = 0; is_safe && ( index < level_cnt - 1 ); ++index )
    {
        // Safe if:
        // 1) the values are steadily increasing or steadily decreasing
        is_safe = ( start_direction == ( levels[ index ] < levels[ index + 1 ] ) );

        // 2) consecutive values are different
        if( is_safe )
        {
            is_safe = ( levels[ index ] != levels[ index + 1 ] );
        }

        // 3) the increment between values is less than or equal to 3
        if( is_safe )
        {
            is_safe = ( std::abs( levels[ index ] - levels[ index + 1 ] ) <= 3 );
        }
    }

    return is_safe;
}

static size_t count_safe(
    std::vector<std::vector<int32_t>>& reports,
    std::vector<size_t>& problem_idx )
{
    // A report is valid if:
    // 1) the levels are all increasing or all decreasing, and
    // 2) adjacent levels differ by at least one and at most three.
    size_t safe_reports = 0;
    for( size_t idx = 0; idx < reports.size(); ++idx )
    {
        if( is_report_safe( reports[ idx ] ) )
        {
            ++safe_reports;
        }
        else
        {
            // Note the report index in the problem list.
            problem_idx.emplace_back( idx );
        }
    }

    return safe_reports;
}

static size_t find_bad_index( std::vector<int32_t>& levels )
{
    size_t bad_idx = std::numeric_limits<size_t>::max();
    bool found = false;

    std::map<size_t, std::pair<int32_t, int32_t>> increasing_path;
    std::map<size_t, std::pair<int32_t, int32_t>> decreasing_path;

    for( size_t idx = 0; idx < levels.size() - 1; ++idx )
    {
        // Make pairs of valid target levels for each level in the record.
        std::pair<int32_t, int32_t> inc_pair{ -1, -1 };
        std::pair<int32_t, int32_t> dec_pair{ -1, -1 };

        int32_t step1 = levels[ idx + 1 ] - levels[ idx ];
        bool direction = ( step1 > 0 );
        if( ( -3 <= step1 && step1 <= -1 ) || ( 1 <= step1 && step1 <= 3 ) )
        {
            std::pair<int32_t, int32_t>& insert_pair = direction ? inc_pair : dec_pair;
            insert_pair.first = static_cast<int32_t>( idx + 1 );
        }

        if( idx < levels.size() - 2 )
        {
            int32_t step2 = levels[ idx + 2 ] - levels[ idx ];
            direction = ( step2 > 0 );
            if( ( -3 <= step2 && step2 <= -1 ) || ( 1 <= step2 && step2 <= 3 ) )
            {
                std::pair<int32_t, int32_t>& insert_pair = direction ? inc_pair : dec_pair;
                insert_pair.second = static_cast< int32_t >( idx + 2 );
            }
        }

        if( ( -1 != inc_pair.first ) || ( -1 != inc_pair.second ) )
        {
            increasing_path[ idx ] = inc_pair;
        }

        if( ( -1 != dec_pair.first ) || ( -1 != dec_pair.second ) )
        {
            decreasing_path[ idx ] = dec_pair;
        }
    }

    if( ( increasing_path.size() < levels.size() - 2 ) &&
        ( decreasing_path.size() < levels.size() - 2 ) )
    {
        // No way to salvage this one
        found = true;
    }

    if( !found )
    {
        // Paths are created, look for valid path.
        // This block will always set bad_idx.
        found = true;

        // Add dummy map entries for the last level targets
        if( increasing_path.size() == levels.size() - 2 )
        {
            std::map<size_t, std::pair<int32_t, int32_t>>::reverse_iterator iter = increasing_path.rbegin();
            size_t idx = ( *iter ).first;
            std::pair<int32_t, int32_t> dummy = ( *iter ).second;

            if( -1 != dummy.first )
            {
                if( is_report_safe( { levels[ idx ], levels[ dummy.first ] } ) &&
                    ( levels[ idx ] < levels[ dummy.first ] ) )
                {
                    increasing_path[ dummy.first ] = std::make_pair( -1, -1 );
                }
            }

            if( -1 != dummy.second )
            {
                if( is_report_safe( { levels[ idx ], levels[ dummy.second ] } ) &&
                    ( levels[ idx ] < levels[ dummy.second ] ) )
                {
                    increasing_path[ dummy.second ] = std::make_pair( -1, -1 );
                }
            }
        }

        if( decreasing_path.size() == levels.size() - 2 )
        {
            std::map<size_t, std::pair<int32_t, int32_t>>::reverse_iterator iter = decreasing_path.rbegin();
            size_t idx = ( *iter ).first;
            std::pair<int32_t, int32_t> dummy = ( *iter ).second;

            if( -1 != dummy.first )
            {
                if( is_report_safe( { levels[ idx ], levels[ dummy.first ] } ) &&
                    ( levels[ idx ] > levels[ dummy.first ] ) )
                {
                    decreasing_path[ dummy.first ] = std::make_pair( -1, -1 );
                }
            }

            if( -1 != dummy.second )
            {
                if( is_report_safe( { levels[ idx ], levels[ dummy.second ] } ) &&
                    ( levels[ idx ] > levels[ dummy.second ] ) )
                {
                    decreasing_path[ dummy.second ] = std::make_pair( -1, -1 );
                }
            }
        }

        std::map<size_t, std::pair<int32_t, int32_t>>& map =
            ( increasing_path.size() > decreasing_path.size() ) ?
                increasing_path :
                decreasing_path;

        std::map<size_t, std::pair<int32_t, int32_t>>::iterator next = map.begin();
        std::map<size_t, std::pair<int32_t, int32_t>>::iterator end = map.end();
        std::map<size_t, std::pair<int32_t, int32_t>>::iterator cur = next;
        ++next;

        // If index zero is not the first entry in the map, then the first
        // level is the bad one.
        if( (*cur).first != 0 )
        {
            bad_idx = 0;
        }

        // Walk the map of valid paths and extract the invalid level index.
        if( std::numeric_limits<size_t>::max() == bad_idx )
        {
            while( end != next )
            {
                // Couple of situation here:
                // 1) if there is a gap in the level indices in the map keys
                //    then the missing index is the bad one;
                // 2) If a map node does not have a first choice target, then
                //    that node is the bad one.
                if( (*next).first != ( (*cur).first + 1 ) )
                {
                    bad_idx = (*cur).first + 1;
                    break;
                }
                else if( -1 == (*cur).second.first )
                {
                    bad_idx = (*cur).first;
                    break;
                }

                cur = next;
                ++next;
            }
        }

        if( std::numeric_limits<size_t>::max() == bad_idx )
        {
            // The last level is the bad one, remove it.
            bad_idx = levels.size() - 1;
        }
    }

    return bad_idx;
}

static size_t apply_problem_dampener(
    std::vector<std::vector<int32_t>>& reports,
    std::vector<size_t>& problem_idx )
{
    // The problem dampener will remove a single problematic value from each
    // report in an attempt to make the report valid.
    // Need to look ahead and behind to determine which value may be
    // problematic.
    size_t reconciled_reports = 0;
    size_t reconciled_reports_bf = 0;

    std::vector<size_t> unresolved_idx;
    std::vector<size_t>::iterator iter = problem_idx.begin();
    while( problem_idx.end() != iter )
    {
        std::vector<int32_t> levels = reports[ *iter ];

        // Find a bad index
        size_t bad_idx = find_bad_index( levels );

#ifdef _DEBUG
        // Brute force it for validation purposes
        size_t bad_idx_bf = -1;
        bool is_reconcilable = false;
        for( size_t idx = 0; idx < reports[ *iter ].size(); ++idx )
        {
            levels.erase( std::next( levels.begin(), idx ) );
            if( is_report_safe( levels ) )
            {
                bad_idx_bf = idx;
                is_reconcilable = true;
                ++reconciled_reports_bf;
                break;
            }

            levels = reports[ *iter ];
        }

        // Does the logic agree with brute force?
        if( is_reconcilable &&
            std::numeric_limits<size_t>::max() == bad_idx )
        {
            // Brute force method says it is unrepairable.
            // Stop here and debug the logic.
            __debugbreak();
        }

        // Reset the report for further processing.
        levels = reports[ *iter ];
#endif

        if( std::numeric_limits<size_t>::max() != bad_idx )
        {
            // Remove the problematic value...
            levels.erase( std::next( levels.begin(), bad_idx ) );

            // Check if modified report is valid
            if( is_report_safe( levels ) )
            {
                // Report is now safe, remove the index from the problem list
                ++reconciled_reports;
                iter = problem_idx.erase( iter );
                continue;
            }
            else
            {
                unresolved_idx.emplace_back( *iter );
            }
        }

        ++iter;
    }

    return reconciled_reports;
}

int main()
{
    // Step 1: Read the input CSV
    std::vector<std::vector<int32_t>> reports;
    std::vector<size_t> problem_idx;

    if( !read_input( reports ) )
    {
        return -1;
    }

    // Step 2: Count valid reports
    size_t safe_reports = count_safe( reports, problem_idx );
    std::cout << "Valid report count = " << safe_reports << "\n";

    // Step 3: Apply the Problem Dampener and recount
    safe_reports += apply_problem_dampener( reports, problem_idx );

    std::cout << "Valid report count after Problem Dampener = " << safe_reports << std::endl;
}
