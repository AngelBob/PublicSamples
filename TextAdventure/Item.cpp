#include "stdafx.h"
#include "Objects.h"

Item::Item( const json &item )
	: InGameObject( item )
{
    // Bail early on invalid object
    if( IsInvalid( item ) )
    {
        return;
    }

    // Build the response lists
    auto &responses = item.at( "Responses" );
    for( auto it = responses.begin(); it != responses.end(); ++it )
    {
    }

    // These items are optional
    try
    {
        m_IsMovable = item.at( "Moveable" );
    }
    catch( json::out_of_range )
    {
        m_IsMovable = false;
    }
}
