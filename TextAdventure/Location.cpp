#include "stdafx.h"

#include "Character.h"
#include "Item.h"
#include "Location.h"

Location::Location( const json &location, std::weak_ptr<Map> map )
	: GameObject( location, map )
	, m_ShownOnce( false )
{
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

	// The neighbor list is also required
	// Convert the direction strings to Map::MOVE_DIRECTION enum values
	std::vector<std::map<std::string, int32_t>> neighbors = location.at( "Neighbors" );
	for( auto &it : neighbors )
	{
		Map::MoveDirection directionId;
		int32_t neighborId;

		std::map<std::string, int32_t>::const_iterator found;
		if( ( found = it.find( "North" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_NORTH;
			neighborId = found->second;
		}
		else if( ( found = it.find( "NorthEast" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_NORTH_EAST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "East" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_EAST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "SouthEast" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_SOUTH_EAST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "South" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_SOUTH;
			neighborId = found->second;
		}
		else if( ( found = it.find( "SouthWest" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_SOUTH_WEST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "West" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_WEST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "NorthWest" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_NORTH_WEST;
			neighborId = found->second;
		}
		else if( ( found = it.find( "Up" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_UP;
			neighborId = found->second;
		}
		else if( ( found = it.find( "Down" ) ) != it.end() )
		{
			directionId = Map::MoveDirection::MOVE_DOWN;
			neighborId = found->second;
		}
		else
		{
			continue;
		}

		m_Neighbors[ directionId ] = neighborId;
	}
}

void Location::AddCharacter( int32_t characterId )
{
	m_Characters.emplace_back( characterId );
}

void Location::AddItem( int32_t itemId )
{
	m_Items.emplace_back( itemId );
}

bool Location::IsStartPosition( void )
{
	return m_IsStartPosition;
}

std::ostream& Location::PrintDescription( std::ostream &os, bool showLongDesc )
{
	if( !m_ShownOnce || showLongDesc )
	{
		os << m_LongDesc;
		m_ShownOnce = true;
	}
	else
	{
		os << m_ShortDesc;
	}

	return os;
}

std::ostream& Location::PrintCharacters( std::ostream &os )
{
	std::shared_ptr<Map> map = m_Map.lock();
	if( map )
	{
		if( m_Characters.size() )
		{
			std::list<int32_t>::iterator cur = m_Characters.begin();
			std::list<int32_t>::iterator stop = m_Characters.end();
			--stop;

			os << "  The ";
			while( cur != stop )
			{
				std::shared_ptr<Character> &obj = map->GetCharacter( *cur );
				obj->PrintName( os );
				if( 2 != m_Characters.size() )
				{
					os << ", ";
				}
				++cur;
			}

			if( 2 == m_Characters.size() )
			{
				os << " and ";
			}
			std::shared_ptr<Character> &obj = map->GetCharacter( *cur );
			obj->PrintName( os );

			if( m_Characters.size() == 1 )
			{
				os << " is";
			}
			else
			{
				os << " are";
			}
			os << " here.";
		}
	}

	return os;
}

std::ostream &Location::PrintItems( std::ostream &os )
{
	std::shared_ptr<Map> map = m_Map.lock();
	if( map )
	{
		if( m_Items.size() )
		{
			std::list<int32_t>::iterator cur = m_Items.begin();
			std::list<int32_t>::iterator stop = m_Items.end();
			--stop;

			os << "  ";
			if( m_Characters.size() > 1 )
			{
				os << "There are ";
			}
			else
			{
				os << "There is a ";
			}

			while( cur != stop )
			{
				std::shared_ptr<Item> &obj = map->GetItem( *cur );
				obj->PrintDescription( os );
				if( 2 != m_Items.size() )
				{
					os << ", ";
				}
				++cur;
			}

			if( 2 == m_Items.size() )
			{
				os << " and a ";
			}
			std::shared_ptr<Item> &obj = map->GetItem( *cur );
			obj->PrintDescription( os );

			os << " here.";
		}
	}

	return os;
}

int32_t Location::NextLocation( const Map::MoveDirection direction )
{
	int32_t nextLocation = 0;

	std::map<Map::MoveDirection, int32_t>::const_iterator found = m_Neighbors.find( direction );
	if( found != m_Neighbors.end() )
	{
		nextLocation = found->second;
	}

	return nextLocation;
}
