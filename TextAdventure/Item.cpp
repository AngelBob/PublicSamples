#include "stdafx.h"

#include "Item.h"

Item::Item( const json &item, std::weak_ptr<Map> map )
	: GameObject( item, map )
{
    // These entries are required
    m_LongDescription = item.at( "Description" );
    m_Examination = item.at( "Examination" );

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

std::ostream &Item::PrintDescription( std::ostream &os )
{
    os << m_LongDescription;

    return os;
}
