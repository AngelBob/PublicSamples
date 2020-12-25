#include "stdafx.h"

#include "Character.h"
#include "Item.h"
#include "Location.h"
#include "Map.h"

struct ResourceList
{
	const wchar_t *Name;
	uint32_t       Id;
};

void Map::LoadMap( std::weak_ptr<Map> weakThis )
{
	// Load all of the game objects from the JSON descriptions
	LoadJSONResources( weakThis );

	// Place the characters and items in their proper places
	PlaceCharacters();
	PlaceItems();

	// Set the starting location
	SetStartingLocation();
}

void Map::LoadJSONResources( std::weak_ptr<Map> weakThis )
{
	// Need to load up the resources and parse the json contained within.
	// There are three resource types:
	// 1) Locations,
	// 2) Characters, and
	// 3) Items.
	static const ResourceList Resources[ 3 ] = {
		{ L"LOCATIONS", IDR_LOCATIONS1 },
		{ L"CHARACTERS", IDR_CHARACTERS1 },
		{ L"ITEMS", IDR_ITEMS1 },
	};

	for( uint32_t idx = 0; idx < _countof( Resources ); ++idx )
	{
		if( 0 == Resources[ idx ].Id )
		{
			continue;
		}

		HRSRC resFinder = ::FindResource( NULL, Resources[ idx ].Name, MAKEINTRESOURCE( Resources[ idx ].Id ) );
		if( NULL == resFinder )
		{
			continue;
		}

		HGLOBAL resLoader = ::LoadResource( NULL, resFinder );
		if( NULL == resLoader )
		{
			continue;
		}

		PVOID resource = ::LockResource( resLoader );
		if( NULL == resource )
		{
			continue;
		}

		// Convert the raw resource data into json type.
		// Pretty simple, really, create a string stream and stream it into the json
		// using the stream operator (>>).  Nice.
		std::string strResource( static_cast< char * >( resource ) );
		std::stringstream resStream( strResource );

		json items;
		resStream >> items;

		// Now that we have the JSON tree, start building the internal game objects
		switch( Resources[ idx ].Id )
		{
		case IDR_LOCATIONS1:
			// Build location objects
			BuildObjects<Location>( items, weakThis );
			break;

		case IDR_CHARACTERS1:
			// Building character objects
			BuildObjects<Character>( items, weakThis );
			break;

		case IDR_ITEMS1:
			// Building item objects
			BuildObjects<Item>( items, weakThis );
			break;
		}
	}
}

template<typename Obj>
void Map::BuildObjects( const json &objJson, std::weak_ptr<Map> weakThis )
{
	// Iterate through the array of objects; adding each to the appropriate list.
	for( auto it = objJson.begin(); it != objJson.end(); ++it )
	{
		std::shared_ptr<Obj> obj = std::make_shared<Obj>( *it, weakThis );

		if constexpr( std::is_same< Obj, Location >::value )
		{
			m_Locations[ obj->GetObjectId() ] = obj;
		}
		if constexpr( std::is_same< Obj, Character >::value )
		{
			m_Characters[ obj->GetObjectId() ] = obj;
		}
		if constexpr( std::is_same< Obj, Item >::value )
		{
			m_Items[ obj->GetObjectId() ] = obj;
		}
	}
}

void Map::PlaceCharacters( void )
{
	// Go through the list of characters and link the character to a location
	for( auto iter : m_Characters )
	{
		int32_t characterId = iter.first;
		int32_t characterLocation = ( iter.second )->GetLocation();

		std::shared_ptr<Location> &location = GetLocation( characterLocation );
		location->AddCharacter( characterId );
	}
}

void Map::PlaceItems( void )
{
	// Go through the list of items and link the item to a location
	for( auto iter : m_Items )
	{
		int32_t itemId = iter.first;
		int32_t itemLocation = ( iter.second )->GetLocation();

		std::shared_ptr<Location> &location = GetLocation( itemLocation );
		location->AddItem( itemId );
	}
}

void Map::SetStartingLocation( void )
{
	// Go through the list of locations and find the starting point
	for( auto iter : m_Locations )
	{
		if( iter.second->IsStartPosition() )
		{
			m_CurLocation = iter.first;
		}
	}
}
std::shared_ptr<Character> &Map::GetCharacter( int32_t characterId )
{
	return m_Characters.at( characterId );
}

std::shared_ptr<Item> &Map::GetItem( int32_t itemId )
{
	return m_Items.at( itemId );
}

int32_t Map::GetLocationId( void )
{
	return m_CurLocation;
}

std::shared_ptr<Location> &Map::GetLocation( int32_t locationId )
{
	// Default returns the current location
	if( -1 == locationId )
	{
		locationId = m_CurLocation;
	}

	return m_Locations.at( locationId );
}
