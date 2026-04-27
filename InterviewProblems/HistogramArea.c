#include <stdio.h>
#include <stdlib.h>

void build_histogram( int** heights, int num_heights )
{
    *heights = (int*)malloc( num_heights * sizeof(int) );
    for( int i = 0; i < num_heights; ++i )
    {
        (*heights)[i] = rand() % num_heights + 1; // Random height between 1 and num_heights
    }
}

void find_largest_rectangle( void )
{
    #define NUM_HEIGHTS 10
    int* heights;
    build_histogram( &heights, NUM_HEIGHTS );

    int max_area = 0;
    int* stack = (int*)malloc( NUM_HEIGHTS * sizeof(int) );
    int top = -1;

    for( int i = 0; i <= NUM_HEIGHTS; ++i )
    {
        int current_height = ( i == NUM_HEIGHTS ) ? 0 : heights[ i ];

        while( top >= 0 && current_height < heights[ stack[ top ] ] )
        {
            int height = heights[ stack[ top-- ] ];
            int width = ( top == -1 ) ? i : ( i - stack[ top ] - 1 );
            max_area = ( height * width > max_area ) ? height * width : max_area;
        }

        stack[ ++top ] = i;
    }
    free( stack );

    for( int i = 0; i < NUM_HEIGHTS; ++i )
    {
        printf( "%d ", heights[ i ] );
    }
    printf( "\n" );
    free( heights );

    printf( "Largest rectangle area: %d\n", max_area );
}