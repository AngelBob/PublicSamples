// MemberHiding.cpp
// Small app to test class member hiding.
// The base class functions are hidden by the derived class.
// Function calls to a derived class result in output of "cd".
// Easy example for why -Wall and treat warnings as errors is
// a good idea.

#include <iostream>

class CBase
{
public:
    // These functions are hidden in the derived class.
    // -Wall produces warnings as the derived class makes
    // these functions unreferenced.
    void yes( [[maybe_unused]] int i ) { std::cout << "cb" << std::endl; }
    void no( [[maybe_unused]] double d ) {std::cout << "cb" <<std::endl; }
};

class CDerived : public CBase
{
public:
    // This function hides CBase::f(), so as long as the
    // value passed to CDerived:f() can be cast to double
    // this function will take precedence.
    void yes( [[maybe_unused]] double d ) { std::cout << "cd" << std::endl; }

    // What if we do it the other way around?
    void no( [[maybe_unused]] int i ) { std::cout << "cd" << std::endl; }
};

int main()
{
    CDerived cd;
    int i = 1;
    cd.yes(i); // Ok, int casts to double without precision loss.

    double d = 1.35;
    cd.no(d); // Possible loss of data warning.
}
