#pragma once

#include "InGameObject.h"

enum class MoveDirection;
class Map;


class Location : public InGameObject
{
public:
	Location( const json &location, const int32_t globalId );

	// Accessors
	bool IsStartPosition( void );

	void AddObject( int32_t itemId );
	void RemoveObject( int32_t itemId );
	const std::list<int32_t>& GetObjects( void ) const;
	bool HasObject( const int32_t id ) const;

	void SetNeighbor( MoveDirection dir, int32_t nieghborId );
	const std::map<MoveDirection, int32_t>& GetNeighbors( void ) const;

	void SetShownOnce( void );
	bool GetShownOnce( void ) const;

	const std::string& GetAsSeenDescription( void ) const;

	inline ObjectType GetType( void ) const
	{
		return ObjectType::OBJECT_LOCATION;
	}

private:
	static const ObjectType  m_ObjectType = ObjectType::OBJECT_LOCATION;

	// Members specific to the location class
	bool        m_IsStartPosition;
	bool		m_ShownOnce;
	std::string m_AsSeenDesc;

	std::map<MoveDirection, int32_t> m_Neighbors;

	std::list<int32_t> m_Objects;
};
