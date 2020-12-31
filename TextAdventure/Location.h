#pragma once

enum class MoveDirection;
class Map;

enum class LocationDesc
{
	DESCRIPTION_LONG,
	DESCRIPTION_SHORT,
	DESCRIPTION_ASSEEN,
};

class Location : public InGameObject
{
public:
	Location( const json &location );

	// Event handlers
	int32_t OnMove( MoveDirection direction );

	// Accessors
	bool IsStartPosition( void );

	void AddCharacter( int32_t characterId );
	void RemoveCharacter( int32_t characterId );
	const std::list<int32_t>& GetCharacters( void ) const;

	void AddItem( int32_t itemId );
	void RemoveItem( int32_t itemId );
	const std::list<int32_t>& GetItems( void ) const;

	void SetNeighbor( MoveDirection dir, int32_t nieghborId );
	const std::map<MoveDirection, int32_t> &GetNeighbors( void ) const;

	void SetShownOnce( void );

	LocationDesc LongOrShortDescription( void ) const;
	std::ostream &PrintDescription( std::ostream &os, LocationDesc which ) const;

private:
	bool        m_IsStartPosition;
	bool		m_ShownOnce;
	std::string m_ShortDesc;
	std::string m_LongDesc;
	std::string m_AsSeenDesc;
	std::map<MoveDirection, int32_t> m_Neighbors;

	std::list<int32_t> m_Characters;
	std::list<int32_t> m_Items;
};
