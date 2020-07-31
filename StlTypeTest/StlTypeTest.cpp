// StlTypeTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Some standard template library fun with types.
// Compile time type checking for enabled or disabled feature; note that both types must be defined
// in order for this to even compile.

#include <iostream>

// Flag to enable/disable the feature; comment this #define to get the Feature_Disabled code behaviour
#define ENABLE_FEATURE

struct FeatureTypes
{
    typedef unsigned int    myunsignedinttype;
};

template<typename T>
struct Feature_Enabled : T
{
    static inline void GetNumber(void)
    {
        const typename T::myunsignedinttype number = 3;
        std::cout << "The feature is enabled, your number is " << number << "\n";
    }
};

template<typename T>
struct Feature_Disabled : T
{
    static inline void GetNumber(void)
    {
        std::cout << "The feature is disabled, so no number for you!\n";
    }
};

#if defined ENABLE_FEATURE
template<typename T>
using Feature = Feature_Enabled<T>;
#else
template<typename T>
using Feature = Feature_Disabled<T>;
#endif

struct FeatureWithTypes : Feature<FeatureTypes> {};

int main()
{
    using FeatureT         = Feature<FeatureTypes>;
    using FeatureDisabledT = Feature_Disabled<FeatureTypes>;

    if( std::is_same_v<FeatureT, FeatureDisabledT> )
    {
        std::cout << "is_same says the feature is disabled, checking...\n";
    }
    else
    {
        std::cout << "is_same says the feature is enabled, checking...\n";
    }

    FeatureWithTypes myFeature;
    myFeature.GetNumber();
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
