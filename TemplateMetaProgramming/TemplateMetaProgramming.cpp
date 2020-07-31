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
    static const uint64_t result = static_cast<uint64_t>( ( temp + 'A' ) << ( 8 * shift ) )
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

//////////////////////////////////////////////////////////////////////////
/// @brief  Binding table copy is CPU intesive, reduce the amount of data that gets copied
template <typename T>
static inline void DoesTHaveThing( T p )
{
    if constexpr( has_thing<T>::value )
    {
        std::cout << "Thing was present in type T: " << p.thing << std::endl;
    }
    else
    {
        std::cout << "No Thing was present in type T" << std::endl;
    }
}

struct IHaveThing
{
    uint64_t thing;
    IHaveThing() : thing( 16 ) {}
};

struct NoThingHere
{
    uint64_t no_thing;
    NoThingHere() : no_thing( 32 ) {}
};

int main()
{
    // Exmaple 1) Factorial meta-programming test
    std::cout << Factorial<5>::result << "\n";

    // More meta-programming fun!  This time with spreadsheet column letters
    // This meta-program works for columns with up to 7 letters - which is
    // larger than a 32-bit integer, so should be okay for most use cases :-)

    // Column "A" is the first column
    unsigned int len = SpreadSheetColumnNameLength<1>::length;
    uint64_t result = SpreadSheetColumnName<1>::result;
    char* text = reinterpret_cast<char*>( &result );
    std::cout << "   1: " << len << ", " << text << "\n";

    // Column "Z" is the 26th
    len = SpreadSheetColumnNameLength<26>::length;
    result = SpreadSheetColumnName<26>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "  26: " << len << ", " << text << "\n";

    // Column "AA" comes after "Z"
    len = SpreadSheetColumnNameLength<27>::length;
    result = SpreadSheetColumnName<27>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "  27: " << len << ", " << text << "\n";

    // Column "ZZ' is pretty far over there...
    len = SpreadSheetColumnNameLength<702>::length;
    result = SpreadSheetColumnName<702>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << " 702: " << len << ", " << text << "\n";

    // What about "ABC"?
    len = SpreadSheetColumnNameLength<731>::length;
    result = SpreadSheetColumnName<731>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << " 731: " << len << ", " << text << "\n";

    // What the heck is way out here? "BBB" is the answer.
    len = SpreadSheetColumnNameLength<1406>::length;
    result = SpreadSheetColumnName<1406>::result;
    text = reinterpret_cast<char*>( &result );
    std::cout << "1406: " << len << ", " << text << "\n";

    // 3) compile-time structure identification
    IHaveThing a;
    NoThingHere b;

    DoesTHaveThing( a );
    DoesTHaveThing( b );

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
