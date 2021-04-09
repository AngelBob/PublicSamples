#include "stdafx.h"
#include "Objects.h"

void Map::OnLoad( void )
{
	// Create the name to direction enum map
	for( int32_t idx = 0; idx < static_cast<int32_t>( MoveDirection::MAX_DIRECTIONS ); ++idx )
	{
		m_NameToMoveDirectionMap.insert( std::make_pair( m_MoveDirectionToNameMap[ idx ].name, m_MoveDirectionToNameMap[ idx ].type ) );
	}

	// Load all of the game objects from the JSON descriptions
	json jsonLocations;
	LoadLocationResources( jsonLocations );

	// Go through the list of locations and do some setup
	for( auto& loc : m_Locations )
	{
		// Add the map entry - maps Id property to GlobalId value
		m_LocationNameToIdMap.insert( std::make_pair( loc->GetObjectName(), loc->GetObjectId() ) );

		// Set the starting location, if appropriate
		if( static_cast<Location*>( loc.get() )->IsStartPosition() )
		{
			SetLocation( loc->GetObjectId() );
		}
	}

	// Build the map by linking all of the locations to their neighbors.
	// Must happen after m_LocationNameToIdMap is built.
	for( auto& loc : m_Locations )
	{
		auto& responses = loc->GetResponses( ResponseType::RESPONSE_TYPE_MOVE );
		for( auto it = responses.begin(); it != responses.end(); ++it )
		{
			// Convert the direction and destination strings to integer values
			MoveDirection directionId = GetDirectionEnum( ( *it )->GetMoveDirection() );
			int32_t destinationId = m_LocationNameToIdMap.at( ( *it )->GetMoveDestination() );

			std::static_pointer_cast<Location>( loc )->SetNeighbor( directionId, destinationId );
		}
	}
}

void Map::OnMove( const std::string &destination )
{
	int32_t dest = m_LocationNameToIdMap.at( destination );
	SetLocation( dest );
}

Location& Map::GetLocation( int32_t locationId )
{
	// Default returns the current location
	if( -1 == locationId )
	{
		locationId = m_CurLocation;
	}

	return *( static_cast<Location*>( m_Locations.at( locationId ).get() ) );
}

Location& Map::GetLocation( const std::string& name )
{
	return GetLocation( m_LocationNameToIdMap.at( name ) );
}

const std::string &Map::GetDirectionName( MoveDirection direction )
{
	assert( direction >= MoveDirection::MOVE_INVALID && direction < MoveDirection::MAX_DIRECTIONS );
	if( direction < MoveDirection::MOVE_INVALID
		|| direction >= MoveDirection::MAX_DIRECTIONS )
	{
		direction = MoveDirection::MOVE_INVALID;
	}

	return m_MoveDirectionToNameMap[ static_cast< int32_t >( direction ) ].name;
}

MoveDirection Map::GetDirectionEnum( const std::string &name )
{
	MoveDirection direction = MoveDirection::MOVE_INVALID;
	auto found = m_NameToMoveDirectionMap.find( name );
	if( found != m_NameToMoveDirectionMap.end() )
	{
		direction = found->second;
	}
	else
	{
		// Full string not found, look for shortcuts
		// Need a case insensitive comparitor
		typename StringCompareT comparitor;
		if( 0 == comparitor.compare( name, "n" ) )
		{
			direction = MoveDirection::MOVE_NORTH;
		}
		else if( 0 == comparitor.compare( name, "ne" ) )
		{
			direction = MoveDirection::MOVE_NORTH_EAST;
		}
		else if( 0 == comparitor.compare( name, "e" ) )
		{
			direction = MoveDirection::MOVE_EAST;
		}
		else if( 0 == comparitor.compare( name, "se" ) )
		{
			direction = MoveDirection::MOVE_SOUTH_EAST;
		}
		else if( 0 == comparitor.compare( name, "s" ) )
		{
			direction = MoveDirection::MOVE_SOUTH;
		}
		else if( 0 == comparitor.compare( name, "sw" ) )
		{
			direction = MoveDirection::MOVE_SOUTH_WEST;
		}
		else if( 0 == comparitor.compare( name, "w" ) )
		{
			direction = MoveDirection::MOVE_WEST;
		}
		else if( 0 == comparitor.compare( name, "nw" ) )
		{
			direction = MoveDirection::MOVE_NORTH_WEST;
		}
		else if( 0 == comparitor.compare( name, "u" ) )
		{
			direction = MoveDirection::MOVE_UP;
		}
		else if( 0 == comparitor.compare( name, "d" ) )
		{
			direction = MoveDirection::MOVE_DOWN;
		}
	}

	return direction;
}

int32_t Map::FindObject( int32_t objId ) const
{
	// Caller isn't sure where in the world the item might be, so find it.
	Location *ptr = nullptr;
	for( const std::shared_ptr<InGameObject>& loc : m_Locations )
	{
		ptr = static_cast<Location*>( loc.get() );
		if( ptr->HasObject( objId ) )
		{
			break;
		}
	}

	int32_t locId = InGameObject::INVALID;
	if( nullptr != ptr )
	{
		locId = ptr->GetObjectId();
	}

	return locId;
}

// private:
void Map::LoadLocationResources( json& locations )
{
	// Parse the json from the text resources
	ResourceLoader::LoadStringResource( locations, L"LOCATIONS", IDR_LOCATIONS1 );

	// Build location objects
	ResourceLoader::BuildObjects<Location>( locations, m_Locations );
}

void Map::SetLocation( int32_t newLocation )
{
	// Accessed via the OnMove event handler, no need for public access
	m_CurLocation = newLocation;
}
