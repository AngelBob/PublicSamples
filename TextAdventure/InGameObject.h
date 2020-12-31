#pragma once
#include "stdafx.h"
#include "Objects.h"

class InGameObject
{
public:
	InGameObject( const json& objectJson )
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

	__inline bool IsInvalid( const json& objectJson ) const
	{
		return( objectJson.at( "Id" ) == 0 && objectJson.at( "Name" ) == "invalid" );
	}

	__inline int32_t GetObjectId( void ) const
	{
		return m_Id;
	}

	__inline int32_t GetLocation( void ) const
	{
		return m_Location;
	}

	__inline void SetLocation( int32_t location )
	{
		m_Location = location;
	}

	__inline const std::string& GetName( void ) const
	{
		return m_Name;
	}

protected:
	int32_t					m_Id;
	int32_t					m_Location;
	std::string				m_Name;
};
