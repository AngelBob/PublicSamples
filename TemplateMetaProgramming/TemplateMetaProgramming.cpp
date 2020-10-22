// TemplateMetaProgramming.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Fun with template meta-programming
// Two examples:
// 1)  Super easy factorial example
// 2)  Much more interesting convert a spreadsheet column number into a column letter, e.g., 1 = A, 26 = Z, 27 = AA, etc.
//     Note that spreadsheet columns are numbered from 1, not zero (zero-based indexing confuses the masses).

#include <iostream>

// 1) Super easy factorial
template<int N>
struct Factorial
{
    static const int result = N * Factorial<N - 1>::result;
};

template<>
struct Factorial<0>
{
    static const int result = 1;
};

// 2) Super snazzy spreadsheet column number to letter
template <unsigned int C>
struct SpreadSheetColumnNameLength
{
    static const unsigned int length = 1 + SpreadSheetColumnNameLength<( C - 1 ) / 26>::length;
    static_assert( length <= ( sizeof( uint64_t ) - 1 ) );
};

template <>
struct SpreadSheetColumnNameLength<0>
{
    static const unsigned int length = 0;
};

template<unsigned int C>
struct SpreadSheetColumnName
{
    static const unsigned int shift = SpreadSheetColumnNameLength<C>::length - 1;
    static const unsigned int temp = ( C - 1 ) % 26;
    static const uint64_t result = ( static_cast<uint64_t>( temp + 'A' ) << ( 8 * shift ) )
                                   | SpreadSheetColumnName<static_cast<unsigned int>( ( C - temp - 1 ) / 26 )>::result;
};

template<>
struct SpreadSheetColumnName<0>
{
    static const uint64_t result = 0;
};

// 3) has_a_thing
// If it's necessary to do determine if the member "thing" exists in the incoming type T.
// Can use template meta-programming to do this.
// This bit requires C++17 standard to compile (static_assert & if constexpr).
// Didn't come up with this on my own; both methods come from the link here
// see https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
#if 1

template <typename T>
struct has_thing
{
    // Can use any two types here, so long as their sizes are different
    using yes_thing = char;
    using  no_thing = unsigned long;
    static_assert( sizeof( yes_thing ) != sizeof( no_thing ) );

    // Here's the first part of the magic:
    // If the incoming type has member m_RenderTargetMask, then the compiler
    // will choose the first test() function implementation.  If there is no
    // m_RenderTargetMask member, then the second, variadic override will be
    // used.
    template <typename C> static yes_thing test( decltype( C::thing ) );
    template <typename C> static  no_thing test( ... );

    // And the last part of the magic:
    // Check the size of the function return type to determine which override
    // the compiler used, and assign true or false based on if the member is
    // there or not.  Snazzy.
    static const int value = ( sizeof( test< T >( 0 ) ) == sizeof( yes_thing ) );
};

#else

// A much simpler version of the stuff above
template <typename T, typename = int>
struct has_thing : std::false_type {};

template <typename T>
struct has_thing<T, decltype( ( void ) T::thing, 0 )> : std::true_type {};

#endif

template <typename T>
static inline void DoesTHaveThing( T p )
{
    if constexpr( has_thing<T>::value )
    {
        std::cout << "Thing was present in type T: " << p.thing << "\n\t";
    }
    else
    {
        std::cout << "No Thing was present in type T" << "\n\t";
    }
}

struct IHaveThing
{
    uint64_t thing;
    IHaveThing() : thing( 16 )
    {}
};

struct NoThingHere
{
    uint64_t no_thing;
    NoThingHere() : no_thing( 32 )
    {}
};

// 4) compile time bitmask builder
// Sometimes it's necessary to create a range of bits for bit mask operations.
// This template builds the constant mask value.
template<unsigned int M>
struct bit_mask
{
    static const uint64_t low_mask = ( 1ull << ( M - 1 ) ) | bit_mask< M - 1>::low_mask;
    static const uint64_t mask = ( 1ull << M ) | bit_mask< M - 1 >::mask;
};

template<>
struct bit_mask<0>
{
    static const uint64_t low_mask = 0ull;
    static const uint64_t mask = 1ull;
};

template<unsigned int L, unsigned int H>
struct bit_mask_range
{
    static const uint64_t high_mask = bit_mask< H >::mask;
    static const uint64_t low_mask = bit_mask< L >::low_mask;

    static const uint64_t mask = high_mask ^ low_mask;
};

// 5) build a compile time most-significant-bit finder
template<unsigned int _Value>
struct msb
{
    static const int32_t value = 1 + msb< ( _Value >> 1 ) >::value;
};

template<>
struct msb<1>
{
    static const int32_t value = 0;
};

template<>
struct msb<0>
{
    static const int32_t value = -1;
};

// 6) build a power-of-two round-up value at compile time
template<unsigned int _Value>
struct round_up_pow2
{
    static const  int32_t msb   = msb< ( _Value ) >::value;
    static const uint32_t value = ( _Value & ( _Value - 1 ) ) == 0 ?  1 << msb : 1 << ( msb + 1 );
};

template<>
struct round_up_pow2<0>
{
    static const uint32_t value = 0;
};

int main()
{
    // Exmaple 1) Factorial meta-programming test
    std::cout << "Factorial meta-program test:\n\t";
    std::cout << "5 Factorial is " << Factorial<5>::result << "\n";
    std::cout << "\n====\n\n";

    // More meta-programming fun!  This time with spreadsheet column letters
    // This meta-program works for columns with up to 7 letters - which is
    // larger than a 32-bit integer, so should be okay for most use cases :-)

    std::cout << "Spreadsheet column number to letter:\n\t";

    // Column "A" is the first column
    unsigned int len = SpreadSheetColumnNameLength<1>::length;
    uint64_t result = SpreadSheetColumnName<1>::result;
    char* text = reinterpret_cast<char*>( &result );
    std::cout << "         1: " << len << ", " << text << "\n\t";

    // Column "Z" is the 26th
    len = SpreadSheetColumnNameLength<26>::length;
    result = SpreadSheetColumnName<26>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "        26: " << len << ", " << text << "\n\t";

    // Column "AB" is the 28th column
    len = SpreadSheetColumnNameLength<28>::length;
    result = SpreadSheetColumnName<28>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "        28: " << len << ", " << text << "\n\t";

    // Column "ZZ" is pretty far over there...
    len = SpreadSheetColumnNameLength<702>::length;
    result = SpreadSheetColumnName<702>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "       702: " << len << ", " << text << "\n\t";

    // Column "AAA" is pretty far over there, too...
    len = SpreadSheetColumnNameLength<703>::length;
    result = SpreadSheetColumnName<703>::result;
    text = reinterpret_cast< char * >( &result );
    std::cout << "       703: " << len << ", " << text << "\n\t";

    // What about "ABC"?
    len = SpreadSheetColumnNameLength<731>::length;
    result = SpreadSheetColumnName<731>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "       731: " << len << ", " << text << "\n\t";

    // What the heck is way out here? "BBB" is the answer.
    len = SpreadSheetColumnNameLength<1406>::length;
    result = SpreadSheetColumnName<1406>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "      1406: " << len << ", " << text << "\n\t";

    // What column is ULONG MAX?
    len = SpreadSheetColumnNameLength<ULONG_MAX>::length;
    result = SpreadSheetColumnName<ULONG_MAX>::result;
    text = reinterpret_cast< char * >( &result );
    std::cout << ULONG_MAX << ": " << len << ", " << text << "\n";
    std::cout << "\n====\n\n";

    // 3) compile-time structure identification
    std::cout << "Compile-time structure ID:\n\t";
    IHaveThing a;
    NoThingHere b;

    DoesTHaveThing( a );
    DoesTHaveThing( b );
    std::cout << "\n====\n\n";

    // 4) bit mask and bit mask range test
    std::cout << "Bit mask and mask range test:\n\t";
    std::cout << "mask through     0th bit            (0x1): 0x" << std::hex << bit_mask< 0>::mask << "\n\t";
    std::cout << "                 5th               (0x3F): 0x" << std::hex << bit_mask< 5>::mask << "\n\t";
    std::cout << "                31st         (0xFFFFFFFF): 0x" << std::hex << bit_mask<31>::mask << "\n\t";
    std::cout << "                32nd        (0x1FFFFFFFF): 0x" << std::hex << bit_mask<32>::mask << "\n\t";
    std::cout << "                47th     (0xFFFFFFFFFFFF): 0x" << std::hex << bit_mask<47>::mask << "\n\t";
    std::cout << "                48th    (0x1FFFFFFFFFFFF): 0x" << std::hex << bit_mask<48>::mask << "\n\t";
    std::cout << "                63rd (0xFFFFFFFFFFFFFFFF): 0x" << std::hex << bit_mask<63>::mask << "\n\t";

    std::cout << "bit range 0th -  5th               (0x3F): 0x" << std::hex << bit_mask_range< 0, 5>::mask << "\n\t";
    std::cout << "         22nd - 31st         (0xFFC00000): 0x" << std::hex << bit_mask_range<22,31>::mask << "\n\t";
    std::cout << "         30th - 47th     (0xFFFFC0000000): 0x" << std::hex << bit_mask_range<30,47>::mask << "\n\t";
    std::cout << "         46th - 63rd (0xFFFFC00000000000): 0x" << std::hex << bit_mask_range<46,63>::mask << "\n";
    std::cout << "\n====\n\n";

    // 5) find value's most significant bit at compile time
    std::cout << "Power of 2 round up test:\n\t";
    std::cout << "0x00: 0x" << std::hex << msb<0>::value << "\n\t";
    std::cout << "0x01: 0x" << std::hex << msb<1>::value << "\n\t";
    std::cout << "0x02: 0x" << std::hex << msb<2>::value << "\n\t";
    std::cout << "0x10: 0x" << std::hex << msb<0x10>::value << "\n\t";
    std::cout << "0x13: 0x" << std::hex << msb<0x13>::value << "\n\t";
    std::cout << "127:  0x" << std::hex << msb<127>::value << "\n\t";
    std::cout << "\n====\n\n";

    // 6) build a power of 2 rounded up value at compile time
    std::cout << "Power of 2 round up test:\n\t";
    std::cout << "0x00: 0x" << std::hex << round_up_pow2<0>::value << "\n\t";
    std::cout << "0x01: 0x" << std::hex << round_up_pow2<1>::value << "\n\t";
    std::cout << "0x02: 0x" << std::hex << round_up_pow2<2>::value << "\n\t";
    std::cout << "0x10: 0x" << std::hex << round_up_pow2<0x10>::value << "\n\t";
    std::cout << "0x13: 0x" << std::hex << round_up_pow2<0x13>::value << "\n\t";
    std::cout << "127:  0x" << std::hex << round_up_pow2<127>::value << "\n\t";
    std::cout << "\n====\n\n";

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
