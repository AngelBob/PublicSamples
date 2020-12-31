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

	// The as seen description is required
	m_AsSeenDesc = location.at( "AsSeenDesc" );

	// Start position will only be set for one location
	m_IsStartPosition = location.value( "IsStartPosition", false );
}

#pragma region Event handlers
int32_t Location::OnMove( MoveDirection direction )
{
	int32_t nextLocation = INVALID_OBJECT;

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

const std::list<int32_t>& Location::GetCharacters( void ) const
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

bool Location::GetShownOnce( void ) const
{
	return m_ShownOnce;
}

const std::string& Location::GetAsSeenDescription( void ) const
{
	return m_AsSeenDesc;
}

#pragma endregion Accessors

#pragma region Privates

#pragma endregion Privates