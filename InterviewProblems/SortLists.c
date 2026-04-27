#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct _LL_NODE
{
    struct _LL_NODE* next;
    struct _LL_NODE* prev;

    int value;
} LL_NODE;

LL_NODE* create_node( int value )
{
    LL_NODE* node = (LL_NODE*)malloc( sizeof(LL_NODE) );
    if( node )
    {
        node->value = value;
        node->next = NULL;
        node->prev = NULL;
    }
    
    return node;
}

void remove_node( LL_NODE** head, LL_NODE* node )
{
    if( head == NULL || node == NULL )
        return;

    LL_NODE* next_node = node->next;
    if( *head == node )
    {
        *head = next_node;
    }

    LL_NODE* prev_node = node->prev;
    if( next_node != NULL )
    {
        next_node->prev = prev_node;
    }

    if( prev_node != NULL )
    {
        prev_node->next = next_node;
    }

    node->next = NULL;
    node->prev = NULL;
}

void insert_after( LL_NODE* node, LL_NODE* new_node)
{
    if( node == NULL || new_node == NULL )
        return;

    new_node->prev = node;
    new_node->next = node->next;

    if( node->next != NULL )
    {
        node->next->prev = new_node;
    }

    node->next = new_node;
}

void insert_list_sorted( LL_NODE** head, LL_NODE* new_node )
{
    if( head == NULL || new_node == NULL )
        return;

    if( *head == NULL || (*head)->value >= new_node->value )
    {
        new_node->next = *head;
        if( *head != NULL )
        {
            (*head)->prev = new_node;
        }
        *head = new_node;
    }
    else
    {
        LL_NODE* current = *head;
        while( current->next != NULL && current->next->value < new_node->value )
        {
            current = current->next;
        }

        insert_after(current, new_node);
    }
}

void create_random_lists( LL_NODE** lists, int num_lists, int num_nodes_per_list )
{
    // Build a single list with all nodes to ensure they are unique and can be merged later.
    int node_count = num_lists * num_nodes_per_list;
    LL_NODE* nodes = create_node(1);
    LL_NODE* end_node = nodes;
    for( int i = 2; i <= node_count; ++i )
    {
        LL_NODE* new_node = create_node( i );
        insert_after( end_node, new_node );
        end_node = new_node;
    }

    // Randomly distribute the nodes into the specified number of lists.
    for( int i = 0; i < num_lists - 1; ++i )
    {
        LL_NODE* head = NULL;

        for( int j = 0; j < num_nodes_per_list; ++j )
        {
            int node_index = rand() % node_count; // Random value between 0 and node_count - 1
            LL_NODE* new_node = nodes; // Start from the head of the all_nodes list
            for( int k = 0; k < node_index; ++k )
            {
                new_node = new_node->next;
                if( new_node == NULL )
                {
                    printf( "Unexpected end of all_nodes list. This should not happen.\n" );
                    break;
                }
            }
            node_count--;

            // Remove the node from the all_nodes list and insert it into the current list in sorted order
            remove_node( &nodes, new_node );
            insert_list_sorted( &head, new_node );
        }

        lists[i] = head;
    }

    lists[num_lists - 1] = nodes; // The remaining nodes go into the last list
}

LL_NODE* merge_and_sort_lists( LL_NODE** lists, int num_lists )
{
    // This function should merge the linked lists and sort the resulting list.
    // The implementation is left as an exercise for the reader.
    LL_NODE* merged_head = NULL;
    LL_NODE* merged_tail = NULL;

    LL_NODE** current_heads = (LL_NODE**)malloc( num_lists * sizeof(LL_NODE*) );
    if( !current_heads )
    {
        return NULL;
    }

    for( int i = 0; i < num_lists; ++i )
    {
        current_heads[i] = lists[i];
    }

    // Merge the lists
    while( 1 )
    {
        // Find the node with the smallest value
        LL_NODE* min_node = NULL;
        int min_value = INT_MAX;

        for( int i = 0; i < num_lists; ++i )
        {
            if( ( current_heads[i] != NULL ) &&
                ( current_heads[i]->value < min_value ) )
            {
                min_value = current_heads[i]->value;
                min_node = current_heads[i];
            }
        }

        // If no more nodes to merge, break
        if( min_node == NULL )
            break;

        // Remove the node from its list and add it to the merged list
        for( int i = 0; i < num_lists; ++i )
        {
            if( current_heads[i] == min_node )
            {
                remove_node( &current_heads[i], min_node );
                break;
            }
        }

        if( merged_head != NULL )
        {
            insert_after( merged_tail, min_node);
            merged_tail = min_node;
        }
        else
        {
            merged_head = min_node;
            merged_tail = min_node;
        }
    }

    free( current_heads );

    return merged_head;
}

int sort_lists( void )
{
    #define NUM_LISTS 4
    #define NUM_NODES_PER_LIST 10

    LL_NODE* lists[NUM_LISTS] = { NULL, NULL, NULL, NULL };
    create_random_lists( lists, NUM_LISTS, NUM_NODES_PER_LIST );
    for( int i = 0; i < NUM_LISTS; ++i )
    {
        printf( "List %d: ", i + 1 );
        LL_NODE* current = lists[i];
        while( current != NULL )
        {
            printf( "%d ", current->value );
            current = current->next;
        }
        printf("\n");
    }

    LL_NODE* merged_list = merge_and_sort_lists( lists, NUM_LISTS );
    while( merged_list != NULL )
    {
        LL_NODE* cur_node = merged_list;
        printf( "%d ", cur_node->value );
        merged_list = cur_node->next;
        free( cur_node );
    }
    printf("\n");

    return 0;
}