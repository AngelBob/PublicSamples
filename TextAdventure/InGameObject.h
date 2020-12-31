#pragma once
#include "stdafx.h"
#include "Objects.h"

class InGameObject
{
public:
	static const int32_t INVALID_OBJECT = 0;

	InGameObject( const json& objectJson )
	{
		m_Id = objectJson.at( "Id" );
		m_Name = objectJson.at( "Name" );

		// Locations and items have descriptions
		m_Description = objectJson.value( "Description", "" );

		// Every item has an "Examine" response
		auto &responses = objectJson.at( "Responses" );
		for( auto it = responses.begin(); it != responses.end(); ++it )
		{
			if( it->at( "Type" ) == "Examine" )
			{
				m_ExaminationResponse = it->at( "Text" );

				bool requiresPossession = it->value( "RequiresPossession", false );
				m_IsEnvironmentExaminable = !requiresPossession;
			}
			if( it->at( "Type" ) == "Take" )
			{
				m_IsTakeable = true;
				m_IsTakeableTrigger = it->value( "RequiresTrigger", "-none-" );
			}
		}


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

	inline bool IsInvalid( const json& objectJson ) const
	{
		return( objectJson.at( "Id" ) == 0 && objectJson.at( "Name" ) == "invalid" );
	}

	inline bool AllowsEnvironmentExamination( void )
	{
		return m_IsEnvironmentExaminable;
	}

	inline bool IsTakeable( void ) const
	{
		return m_IsTakeable;
	}

	const std::string &GetTakeableTrigger( void ) const
	{
		return m_IsTakeableTrigger;
	}

	inline int32_t GetObjectId( void ) const
	{
		return m_Id;
	}

	inline int32_t GetLocation( void ) const
	{
		return m_Location;
	}

	inline void SetLocation( int32_t location )
	{
		m_Location = location;
	}

	inline const std::string& GetName( void ) const
	{
		return m_Name;
	}

	inline const std::string& GetDescription( void ) const
	{
		return m_Description;
	}

	inline const std::string& GetExaminationResponse( void ) const
	{
		return m_ExaminationResponse;
	}

protected:
	int32_t		m_Id;
	std::string m_Name;
	int32_t		m_Location;

	// Response strings
	std::string m_Description;
	std::vector<std::string> m_InteractionResponses;
	std::string m_ExaminationResponse;

	bool m_IsTakeable;
	std::string m_IsTakeableTrigger;

	bool m_IsEnvironmentExaminable;
};
