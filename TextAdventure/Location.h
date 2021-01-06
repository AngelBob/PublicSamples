#pragma once

#include "InGameObject.h"

enum class MoveDirection;
class Map;


class Location : public InGameObject
{
public:
	Location( const json &location, ObjectType type, const int32_t classId, const int32_t globalId );

	// Event handlers
	int32_t OnMove( MoveDirection direction );

	// Accessors
	bool IsStartPosition( void );

	void AddObject( int32_t itemId );
	void RemoveObject( int32_t itemId );
	const std::list<int32_t>& GetObjects( void ) const;

	void SetNeighbor( MoveDirection dir, int32_t nieghborId );
	const std::map<MoveDirection, int32_t> &GetNeighbors( void ) const;

	void SetShownOnce( void );
	bool GetShownOnce( void ) const;

	const std::string& GetAsSeenDescription( void ) const;

private:
	// Members specific to the location class
	bool        m_IsStartPosition;
	bool		m_ShownOnce;
	std::string m_AsSeenDesc;

	std::map<MoveDirection, int32_t> m_Neighbors;

	std::list<int32_t> m_Objects;
};
