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

	// Build the map by linking all of the locations to their neighbors
	auto mlocIter = m_Locations.begin();
	++mlocIter; // Skip the first invalid location
	for( auto loc = jsonLocations.begin(); loc != jsonLocations.end() && mlocIter != m_Locations.end(); ++loc, ++mlocIter )
	{
		auto &responses = loc->at( "Responses" );
		for( auto it = responses.begin(); it != responses.end(); ++it )
		{
			if( it->at( "Type" ) == "Movement" )
			{
				// Convert the direction strings to Map::MOVE_DIRECTION enum values
				const std::string& direction = it->at( "Direction" );
				MoveDirection directionId = GetDirectionEnum( direction );

				// Convert the destination id to a global location ID
				int32_t destinationId = it->at( "DestinationId" );
				auto found = std::find_if( m_Locations.begin(), m_Locations.end(), [destinationId]( const std::shared_ptr<InGameObject> obj ) { return ( destinationId == obj->GetObjectClassId() );  } );
				if( found != m_Locations.end() )
				{
					static_cast< Location * >( ( *mlocIter ).get() )->SetNeighbor( directionId, (*found)->GetObjectId() );
				}
			}
		}
	}
	
	// Go through the list of locations and do additional setup
	for( auto& location : m_Locations )
	{
		// Add the map entry
		m_LocationNameToIdMap.insert( std::make_pair( location->GetName(), location->GetObjectId() ) );

		// Set the starting location, if appropriate
		if( static_cast<Location*>( location.get() )->IsStartPosition() )
		{
			SetLocation( location->GetLocation() );
		}
	}
}

bool Map::OnMove( const std::string &direction )
{
	bool IsMoveValid = false;

	MoveDirection dir = GetDirectionEnum( direction );
	Location& loc = GetLocation();

	int32_t newLoc = loc.OnMove( dir );
	if( InGameObject::INVALID != newLoc )
	{
		// Valid move
		IsMoveValid = true;
		SetLocation( newLoc );
	}

	return IsMoveValid;
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

// private:
void Map::LoadLocationResources( json& locations )
{
	// Parse the json from the text resources
	ResourceLoader::LoadStringResource( locations, L"LOCATIONS", IDR_LOCATIONS1 );

	// Build location objects
	ResourceLoader::BuildObjects<Location>( locations, ObjectType::OBJECT_LOCATION, m_Locations );
}

void Map::SetLocation( int32_t newLocation )
{
	// Accessed via the OnMove event handler, no need for public access
	m_CurLocation = newLocation;
}
