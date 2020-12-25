#pragma once

#include "GameObject.h"
#include "Map.h"

class Location : public GameObject
{
public:
	Location( const json &location, std::weak_ptr<Map> map );
	bool IsStartPosition( void );
	int32_t NextLocation( const Map::MoveDirection direction );

	void AddCharacter( int32_t characterId );
	std::ostream &PrintCharacters( std::ostream &os );

	void AddItem( int32_t itemId );
	std::ostream &PrintItems( std::ostream &os );

	std::ostream &PrintDescription( std::ostream &os, bool showLongDesc = false );
	
private:
	template<typename ListT, typename FuncT>
	void PrintThings( std::ostream &os, ListT list, FuncT getter );

	bool        m_IsStartPosition;
	bool		m_ShownOnce;
	std::string m_ShortDesc;
	std::string m_LongDesc;
	std::string m_AsSeenDesc;
	std::map<Map::MoveDirection, int32_t> m_Neighbors;

	std::list<int32_t> m_Characters;
	std::list<int32_t> m_Items;
};
