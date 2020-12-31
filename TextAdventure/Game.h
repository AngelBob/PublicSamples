#pragma once

#include "Character.h"
#include "Item.h"
#include "Location.h"
#include "Map.h"

class Game
{
	// The main game object.
	// This object contains the pointers to all of the InGameObject objects
	// as well as the map object.
	struct ResourceList
	{
		const wchar_t* name;
		const int32_t  id;
	};

public:
	// Let the compiler generate the default ctor and dtor
	// until there is a compelling reason not to.

	// Event handlers
	void OnLoad( void );
	void OnUnload( void );

	void OnMove( const ParserT& parser );

	// Accessors
	const Map& GetMap( void );

	// Utility
	void DescribeScene( void );

private:
	void LoadGameResources( void );

	std::ostream &PrintDirectionsAsSeen( std::ostream &os, size_t &numNeighbors ) const;
	std::ostream &PrintCharacters( std::ostream &os, size_t &numCharacters ) const;
	std::ostream &PrintItems( std::ostream &os, size_t &numItems ) const;

	// Map contains a list of locations
	std::unique_ptr<Map>	m_Map;

	// Characters and Items are stand-alone things, but relate to the locations
	std::vector<std::unique_ptr<Character>> m_Characters;
	std::vector<std::unique_ptr<Item>>      m_Items;
};