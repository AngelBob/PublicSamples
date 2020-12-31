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
	auto location = m_Locations.begin();
	++location; // Skip the first invalid location
	for( auto loc = jsonLocations.begin(); loc != jsonLocations.end() && location != m_Locations.end(); ++loc, ++location )
	{
		auto &responses = loc->at( "Responses" );
		for( auto it = responses.begin(); it != responses.end(); ++it )
		{
			if( it->at( "Type" ) == "Movement" )
			{
				// Convert the direction strings to Map::MOVE_DIRECTION enum values
				const std::string& direction = it->at( "Direction" );
				int32_t destinationId = it->at( "DestinationId" );

				MoveDirection directionId = GetDirectionEnum( direction );
				( *location )->SetNeighbor( directionId, destinationId );
			}
		}
	}
	
	// Set the starting location
	// Go through the list of locations and find the starting point
	for( auto &iter : m_Locations )
	{
		if( iter->IsStartPosition() )
		{
			SetLocation( iter->GetLocation() );
		}
	}
}

bool Map::OnMove( const std::string &direction )
{
	bool IsMoveValid = false;

	MoveDirection dir = GetDirectionEnum( direction );
	Location& loc = GetLocation();

	int32_t newLoc = loc.OnMove( dir );
	if( newLoc > 0 )
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

	return *(m_Locations.at( locationId ));
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
		typename Parser::StringCompareNoCase comparitor;
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
	ResourceLoader::BuildObjects( locations, m_Locations );
}

void Map::SetLocation( int32_t newLocation )
{
	// Accessed via the OnMove event handler, no need for public access
	m_CurLocation = newLocation;
}
