extern void sort_lists( void );
extern void find_largest_rectangle( void );
extern void count_pairs( unsigned int );

int main()
{
    sort_lists();

    find_largest_rectangle();

    count_pairs( 0 );
    count_pairs( 1 );
    count_pairs( 2 );
    count_pairs( 3 );

    return 0;
}