#include "stdafx.h"
#include "Objects.h"

Location::Location( const json &location )
	: InGameObject( location )
	, m_ShownOnce( false )
{
	// Bail on invalid location creation
	if( IsInvalid( location ) )
	{
		return;
	}

	// These entries are required
	m_ShortDesc = location.at( "ShortDesc" );
	m_LongDesc = location.at( "LongDesc" );

	// These entries are optional
	try
	{
		m_AsSeenDesc = location.at( "AsSeenDesc" );
	}
	catch( json::out_of_range )
	{
		m_AsSeenDesc = "";
	}

	try
	{
		m_IsStartPosition = location.at( "IsStartPosition" );
	}
	catch( json::out_of_range )
	{
		m_IsStartPosition = false;
	}
}

#pragma region Event handlers
int32_t Location::OnMove( MoveDirection direction )
{
	int32_t nextLocation = -1;

	std::map<MoveDirection, int32_t>::const_iterator found = m_Neighbors.find( direction );
	if( found != m_Neighbors.end() )
	{
		nextLocation = found->second;
	}

	return nextLocation;
}
#pragma endregion Event handlers

#pragma region Accessors
bool Location::IsStartPosition( void )
{
	return m_IsStartPosition;
}

void Location::AddCharacter( int32_t characterId )
{
	m_Characters.emplace_back( characterId );
}

void Location::RemoveCharacter( int32_t characterId )
{
	m_Characters.remove_if( [ characterId ]( int n ) { return n == characterId; } );
}

const std::list<int32_t> &Location::GetCharacters( void ) const
{
	return m_Characters;
}

void Location::AddItem( int32_t itemId )
{
	m_Items.emplace_back( itemId );
}

void Location::RemoveItem( int32_t itemId )
{
	m_Items.remove_if( [itemId]( int n ) { return n == itemId; } );
}

const std::list<int32_t> &Location::GetItems( void ) const
{
	return m_Items;
}

void Location::SetNeighbor( MoveDirection dir, int32_t neighborId )
{
	m_Neighbors.insert( std::make_pair( dir, neighborId ) );
}

const std::map<MoveDirection, int32_t>& Location::GetNeighbors( void ) const
{
	return m_Neighbors;
}

void Location::SetShownOnce( void )
{
	m_ShownOnce = true;
}

LocationDesc Location::LongOrShortDescription( void ) const
{
	LocationDesc descType = LocationDesc::DESCRIPTION_SHORT;
	if( !m_ShownOnce )
	{
		descType = LocationDesc::DESCRIPTION_LONG;
	}

	return descType;
}

std::ostream& Location::PrintDescription( std::ostream &os, LocationDesc which ) const
{
	switch( which )
	{
	case LocationDesc::DESCRIPTION_LONG:
		os << m_LongDesc;
		break;
	case LocationDesc::DESCRIPTION_SHORT:
		os << m_ShortDesc;
		break;
	case LocationDesc::DESCRIPTION_ASSEEN:
		os << m_AsSeenDesc;
		break;
	}

	return os;
}
#pragma endregion Accessors

#pragma region Privates

#pragma endregion Privates