#pragma once
#include "stdafx.h"

class Map;

class GameObject
{
public:
	GameObject( const json &objectJson, std::weak_ptr<Map> map )
		: m_Map( map )
	{
		m_Id = objectJson.at( "Id" );
		m_Name = objectJson.at( "Name" );
		try
		{
			m_Location = objectJson.at( "Location" );
		}
		catch( json::out_of_range )
		{
			// Locations don't have a location, just default it to the ID value
			m_Location = m_Id;
		}
	}

	int32_t GetObjectId( void )
	{
		return m_Id;
	}

	int32_t GetLocation( void )
	{
		return m_Location;
	}

	void SetLocation( int32_t location )
	{
		m_Location = location;
	}

	std::ostream& PrintName( std::ostream &os )
	{
		os << m_Name;

		return os;
	}

	static void from_json( const json &objectJson, GameObject &go )
	{
	}
protected:
	int32_t					m_Id;
	int32_t					m_Location;
	std::string				m_Name;

	std::weak_ptr<Map>      m_Map;
};
