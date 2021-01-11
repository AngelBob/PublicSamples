#include "stdafx.h"
#include "Objects.h"

Location::Location( const json &location, const int32_t globalId )
	: InGameObject( location, globalId )
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

#pragma region Accessors
bool Location::IsStartPosition( void )
{
	return m_IsStartPosition;
}

void Location::AddObject( int32_t itemId )
{
	m_Objects.emplace_back( itemId );
}

void Location::RemoveObject( int32_t itemId )
{
	m_Objects.remove_if( [itemId]( int n ) { return n == itemId; } );
}

const std::list<int32_t>& Location::GetObjects( void ) const
{
	return m_Objects;
}

bool Location::HasObject( const int32_t id ) const
{
	std::list<int32_t>::const_iterator it = std::find( m_Objects.begin(), m_Objects.end(), id );

	return ( it != m_Objects.end() );
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