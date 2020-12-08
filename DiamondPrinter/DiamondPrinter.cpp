// DiamondPrinter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

void line_printer_dual_loop( int edge_length )
{
    // Upper outer loop - print the top half of the diamond
    for( int line = 0; line < edge_length; ++line )
    {
        // Inner loop A - print the leading spaces
        for( int space = 0; space < edge_length - line - 1; ++space )
        {
            std::cout << " ";
        }

        // Staticly print a single start
        std::cout << "*";

        // Inner loop B - print the inner spaces
        if( line > 0 )
        {
            for( int star = 0; star < ( line * 2 ) - 1; ++star )
            {
                std::cout << " ";
            }

            // Statically print the second star
            std::cout << "*";
        }

        std::cout << "\n";
    }

    // Lower outer loop - print the bottom half of the diamond
    for( int line = 1; line < edge_length; ++line )
    {
        // Inner loop A - print the leading spaces
        for( int space = 0; space < line; ++space )
        {
            std::cout << " ";
        }

        // Staticly print a single start
        std::cout << "*";

        // Inner loop B - print the inner spaces
        if( line < edge_length - 1 )
        {
            for( int star = 0; star < ( ( edge_length * 2 ) - 1 ) - ( line * 2 ) - 2; ++star )
            {
                std::cout << " ";
            }

            // Statically print the second star
            std::cout << "*";
        }

        std::cout << "\n";
    }
}

// Leverage the fact that the number of spaces required per line can be represented by linear equations.
// Solution calculates the equations for the four lines (why four?), then solves those equations to
// get the number of leading and inner spaces per line.
void line_printer_single_loop( int edge_length )
{
    // Outer loop - print each line of the diamond
    for( int x = 0; x < ( edge_length * 2 ) - 1; ++x )
    {
        // The diamond leading edge moves out/in by one space per line, so slope is simply -/+1
        // The intercepts are correspondingly +/-( edge length - 1 )
        int m0 = ( x < edge_length ) ? -1 : 1;
        int b0 = ( x < edge_length ) ? edge_length - 1 : -( edge_length - 1 );
        int y0 = ( m0 * x ) + b0;

        // The trailing diamond edge moves out/in by one space, but the leading edge has also moved
        // by one space, so need to add or remove two inner spaces per line, so slope is simply +/-2
        // The intercepts are a bit harder; the intercept for the positively sloped portion
        // of the graph is at -1 (draw it out), but the intercept for the negatively
        // sloped portion of the graph must be calculated using the point slope
        // formula and solving for b at the point y, edge_length - 1 (that point
        // is on both lines):
        // y = m*x + b; where:
        //    m = -2                  (the slope of the line in which we are interested)
        //    x = ( edge_length - 1 ) (the one point that's on both lines)
        //    y = ( edge_length - 1 ) * 2 - 1, (the y value for this point on both lines)
        // Solve for b ( b = y - m * x ) :  b = ( 4 * edge_length ) - 5
        int m1 = x < edge_length ?  2 : -2;
        int b1 = x < edge_length ? -1 : ( 4 * edge_length ) - 5;
        int y1 = ( m1 * x ) + b1;

        // Inner loop A - print the leading spaces
        for( int space = 0; space < y0; ++space )
        {
            std::cout << " ";
        }

        // Staticly print a single star
        std::cout << "*";

        // Inner loop B - print the inner spaces
        if( x > 0 && x < ( edge_length * 2 ) - 2 )
        {
            for( int space = 0; space < y1; ++space )
            {
                std::cout << " ";
            }

            // Statically print the second star
             std::cout << "*";
        }

        std::cout << "\n";
    }
}

// Re-imagined version of the linear equation solution.
// Takes the linear equation solution and refactors to a single equation per
// loop that uses absolute values to create the "V" shape
void line_printer_single_loop_abs( int edge_length )
{
    // Outer loop - print each line of the diamond
    for( int x = 0; x < ( edge_length * 2 ) - 1; ++x )
    {
        // The top half of the leading edge of the diamond moves to the left by one space per line.
        // The slope of the leading spaces count line is then -1.
        // The number of spaces required at the first line is ( edge length - 1 ), which is the intercept.
        // The slope/intercept equation is:
        //    y = -x + ( edge_length - 1 ).
        // This equation "goes negative" at the corner of the diamond (@ edge_length - 1), so we
        // want to turn back positive at that point.  Can use ABS for doing that.
        // Re-ordering this equation and taking the ABS gives:
        //    y = abs( edge_length - x - 1 )
        int y0  = abs( edge_length - x - 1 );

        // Inner loop A - print the leading spaces
        for( int space = 0; space < y0; ++space )
        {
            std::cout << " ";
        }

        // Staticly print a single star at the leading edge (every line needs at least one star)
        std::cout << "*";

        // The top half of the trailing diamond edge moves to the right by one space per line, but
        // the leading edge has also moved by to the left by one space, so must add two inner
        // spaces per line.  The slope for this equation is +2.
        // The intercept is a bit harder, but drawing it out shows the intercept for the
        // positively sloped portion of the graph is at -1.
        // The slope/intercept equation is y = 2x - 1
        // This equation is never negative on the range of interest, so simply taking ABS
        // fails here.  It is necessary to apply a transform to get the correct V shape.
        // That transform is:
        // a) subtract the value at the "peak of the V" to shift half of the line segment into
        //    negative values, then
        // b) take the abs of the shifted equation to create the V shape, but this "V" is inverted, so
        // c) negate the equation to correct the inversion, and finally
        // d) add back the value at the peak of the V to shift the whole V back into the
        //    correct position.
        //
        // Here it is:
        // a) subtract the "peak of the V" (the maximum number of spaces that need to be inserted)
        //    which is:  ( 2 * edge_length ) - 3.  The resulting equation is:
        //       y = 2x - 1 - ( 2 * edge_length - 3 )
        // b), take the ABS:
        //       y = abs( 2x - 1 - ( ( 2 * edge_length ) - 3 ) )
        //    combine the constants:
        //       y = abs( 2x - ( 2 * edge_length ) + 2 )
        //    factor out the 2:
        //       y = 2 * abs( x - edge_length + 1 ).
        // c) negate the whole thing:
        //       y = -2 * abs( x - edge_length + 1 )
        // d) re-add the offset value:
        //       y = ( -2 * abs( x - edge_length + 1 ) ) + ( 2 * edge_length ) - 3
        int y1     = ( -2 * abs( x - edge_length + 1 ) ) + ( 2 * edge_length ) - 3;

        // Inner loop B - print the inner spaces
        for( int space = 0; space < y1; ++space )
        {
            std::cout << " ";
        }

        // Statically print the second star for lines the require it (not all lines require 2 stars)
        if( x > 0 && x < ( ( edge_length * 2 ) - 2 ) )
        {
            std::cout << "*";
        }

        std::cout << "\n";
    }
}

int main()
{
    // Get the user input...
    int edge_length;
    std::cout << "Enter the side length: ";
    std::cin >> edge_length;

    //std::cout << "Dual-loop print:\n";
    //line_printer_dual_loop( edge_length );

    //std::cout << "Single loop print:\n";
    //line_printer_single_loop( edge_length );

    std::cout << "Single loop abs print:\n";
    line_printer_single_loop_abs( edge_length );
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
