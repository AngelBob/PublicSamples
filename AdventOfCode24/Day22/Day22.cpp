// Day22.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <tuple>
#include <vector>

//#define USE_MAP

static bool read_input(
    std::vector<uint32_t>& secrets
)
{
    // Open the input file and read the data.
    // Step 1: open the input file.
    std::ifstream file(".\\Secrets.txt");

    // Step 2: read each line and insert individual characters into the grid.
    std::string line;
    while (std::getline(file, line))
    {
        secrets.emplace_back( std::stoul( line ) );;
    }

    // Step 3: return success or failure.
    return true;
}

static uint32_t calculate_secret(
    const uint32_t val
)
{
    uint32_t next;

    // Multiply by 64, mix, and prune.
    next = ( ( val << 6 ) ^ val ) & 0xFFFFFF;

    // Divide by 32 and mix
    next ^= ( next >> 5 );

    // Prune
    next &= 0xFFFFFF;

    // Multiply by 2048 and mix
    next ^= ( next << 11 );

    // Final prune
    next &= 0xFFFFFF;
    
    return next;
}

#if defined( USE_MAP )

static std::tuple<int8_t, int8_t, int8_t, int8_t> create_tuple(
    const std::list<int8_t>& val_list
)
{
    size_t idx = 0;
    std::array<int8_t, 4> vec;
    for( const int8_t val : val_list )
    {
        vec[ idx++ ] = val;
    }
    return std::apply( []( auto... elems ) { return std::tuple_cat( ( std::tuple{ elems } )... ); }, vec);
}

static void build_vendor_cues(
    const std::vector<uint32_t>& secrets,
    uint64_t& secret_sum,
    std::vector<std::map<std::tuple<int8_t, int8_t, int8_t, int8_t>, uint8_t>>& vendor_cues
)
{
    vendor_cues.reserve( secrets.size() );

    size_t idx = 0;
    for( uint32_t secret : secrets )
    {
        std::map<std::tuple<int8_t, int8_t, int8_t, int8_t>, uint8_t> this_cue;
        vendor_cues.push_back( this_cue );

        std::list<int8_t> cue_stack;

        for( size_t round = 0; round < 2000; ++round )
        {
            int8_t last_price = secret % 10;
            secret = calculate_secret(secret);
            int8_t this_price = secret % 10;

            cue_stack.emplace_back( this_price - last_price );
            if( 5 == cue_stack.size() )
            {
                cue_stack.pop_front();
            }

            if( 4 == cue_stack.size() )
            {
                vendor_cues[ idx ].try_emplace( create_tuple( cue_stack ), this_price );
            }
        }

        ++idx;
        secret_sum += secret;
    }
}

static uint64_t find_max_cue_sum(
    std::vector<std::map<std::tuple<int8_t, int8_t, int8_t, int8_t>, uint8_t>>& vendor_cues
)
{
    uint64_t max_sale = 0;
    std::tuple<int8_t, int8_t, int8_t, int8_t> max_sale_cue;

    std::map<std::tuple<int8_t, int8_t, int8_t, int8_t>, uint64_t> max_sales;
    for( auto& vendor_cue : vendor_cues )
    {
        auto sell_cue = vendor_cue.begin();
        while( vendor_cue.end() != sell_cue )
        {
            max_sales[sell_cue->first ] += sell_cue->second;
            if( max_sale < max_sales[ sell_cue->first ] )
            {
                max_sale = max_sales[ sell_cue->first ];
                max_sale_cue = sell_cue->first;
            }

            ++sell_cue;
        }
    }

    return max_sale;
}

#else

static void find_max_cue_sales(
    const std::vector<uint32_t>& secrets,
    uint16_t* cue_sales,
    uint64_t& secrect_sum,
    uint64_t& max_sale
)
{
    secrect_sum = 0;
    max_sale = 0;

    for( uint32_t secret : secrets )
    {
        std::array<size_t, 5> cue_indices;
        size_t idx = 0;

        const size_t is_set_count = static_cast<size_t>( std::pow( 19, 4 ) );
        std::unique_ptr<bool[]> is_set = std::make_unique<bool[]>( is_set_count );
        ::memset( is_set.get(), 0, is_set_count * sizeof( bool ) );

        for (size_t round = 0; round < 2000; ++round)
        {
            int8_t last_price = secret % 10;
            secret = calculate_secret(secret);
            int8_t this_price = secret % 10;

            cue_indices[ idx++ ] = ( this_price - last_price ) + 9;
            if( 5 == idx )
            {
                --idx;
                cue_indices[ 0 ] = cue_indices[ 1 ];
                cue_indices[ 1 ] = cue_indices[ 2 ];
                cue_indices[ 2 ] = cue_indices[ 3 ];
                cue_indices[ 3 ] = cue_indices[ 4 ];
            }

            if( 4 == idx )
            {
                size_t idx = 0;
                idx += cue_indices[ 0 ] * static_cast<size_t>( std::pow( 19, 3 ) );
                idx += cue_indices[ 1 ] * static_cast<size_t>( std::pow( 19, 2 ) );
                idx += cue_indices[ 2 ] * static_cast<size_t>( std::pow( 19, 1 ) );
                idx += cue_indices[ 3 ] * static_cast<size_t>( std::pow( 19, 0 ) );

                if( !is_set.get()[ idx ] )
                {
                    is_set.get()[ idx ] = true;
                    cue_sales[ idx ] += this_price;
                    if( max_sale < cue_sales[ idx ] )
                    {
                        max_sale = cue_sales[ idx ];
                    }
                }
            }
        }

        secrect_sum += secret;
    }
}
#endif

int main()
{
    std::vector<uint32_t> secrets;
    if( !read_input( secrets ) )
    {
        return -1;
    }

#if defined( USE_MAP )
    uint64_t secret_sum = 0;
    std::vector<std::map<std::tuple<int8_t, int8_t, int8_t, int8_t>, uint8_t>> vendor_cues;
    build_vendor_cues( secrets, secret_sum, vendor_cues );

    std::cout << "The sum of all secrets is: " << std::to_string( secret_sum ) << "\n";

    uint64_t max_sum = find_max_cue_sum( vendor_cues );
    std::cout << "The best sell cue nets " << std::to_string( max_sum ) << std::endl;
#else
    uint64_t secret_sum = 0;
    uint64_t max_sum = 0;

    size_t cue_sales_count = static_cast<size_t>( std::pow( 19, 4 ) );
    std::unique_ptr<uint16_t[]> cue_sales = std::make_unique<uint16_t[]>( cue_sales_count );
    ::memset( cue_sales.get(), 0, cue_sales_count * sizeof( uint16_t ) );

    find_max_cue_sales( secrets, cue_sales.get(), secret_sum, max_sum );
    std::cout << "The sum of all secrets is: " << std::to_string( secret_sum ) << "\n";
    std::cout << "The best sell cue nets " << std::to_string( max_sum ) << std::endl;
#endif
}
