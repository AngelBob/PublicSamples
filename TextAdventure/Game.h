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
	void OnTake( const ParserT &parser );
	void OnExamine( const ParserT& parser );
	void OnInteraction( const ParserT &parser );

	void OnInventory( void ) const;

	// Accessors
	const Map& GetMap( void );

	// Utility
	void DescribeScene( void );

private:
	void LoadGameResources( void );

	std::ostream &PrintDirectionsAsSeen( std::ostream &os, size_t &numNeighbors ) const;
	std::ostream &PrintCharacters( std::ostream &os, size_t &numCharacters ) const;
	std::ostream &PrintItems( std::ostream &os, size_t &numItems ) const;

	bool GetItemData( const std::string &name, int32_t &itemId, int32_t& itemLocId, bool& isInInventory ) const;
	void DoItemExamination( const std::string &name, int32_t itemId, bool isInInventory ) const;

	bool GetCharacterData( const std::string &name, int32_t& characterId, int32_t& charLocId, bool& isInPosse ) const;
	void DoCharacterExamination( const std::string &name, int32_t characterId, bool isInPosse ) const;

	// Map contains a list of locations
	std::unique_ptr<Map>	m_Map;

	// Characters and Items are stand-alone things, but relate to the locations
	std::map<std::string, int32_t, typename StringCompareT> m_CharacterNameToIdMap;
	std::vector<std::unique_ptr<Character>> m_Characters;

	std::map<std::string, int32_t, StringCompareT>	m_ItemNameToIdMap;
	std::vector<std::unique_ptr<Item>>      m_Items;

	// The user inventory
	std::list<int32_t> m_Inventory; // It's possible to collect items
	std::list<int32_t> m_Posse;     // It's possible to collect characters
};