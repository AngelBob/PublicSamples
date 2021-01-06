#pragma once
#include "stdafx.h"
#include "Objects.h"

#pragma region Response Object
Response::Response( void )
	: m_RequiresPossession( false )
	, m_RequiresLocation( 0 )
	, m_CurrentResponse( 0 )
{
}

bool Response::ObjectPossessionIsRequired( void ) const
{
	return m_RequiresPossession;
}

const std::list<int32_t>& Response::GetRequiredObjects( void ) const
{
	return m_RequiresObjects;
}

int32_t Response::GetRequiredLocation( void ) const
{
	return m_RequiresLocation;
}

const std::string& Response::GetRequiredVerb( void ) const
{
	return m_RequiresVerb;
}

const std::string& Response::GetRequiredEvent( void ) const
{
	return m_RequiresEvent;
}

const std::string& Response::GetTriggeredEvent( void ) const
{
	return m_TriggersEvent;
}

std::string Response::GetResponseText( void )
{
	if( 0 < m_Text.size() )
	{
		const std::string &text = m_Text.at( m_CurrentResponse++ );

		if( m_CurrentResponse >= m_Text.size() )
		{
			m_CurrentResponse = m_Text.size() - 1;
		}

		return text;
	}
	else
	{
		return "";
	}
}

void Response::SetRequiresPossession( bool required )
{
	m_RequiresPossession = required;
}

void Response::PushRequiredObject( int32_t id )
{
	m_RequiresObjects.emplace_back( id );
}

void Response::SetRequiredLocation( int32_t id )
{
	m_RequiresLocation = id;
}

void Response::SetRequiredVerb( const std::string &verb )
{
	m_RequiresVerb = verb;
}

void Response::SetRequiredEvent( const std::string &event )
{
	m_RequiresEvent = event;
}

void Response::SetTriggeredEvent( const std::string &event )
{
	m_TriggersEvent = event;
}

void Response::PushResponseText( const std::string &text )
{
	m_Text.emplace_back( text );
}
#pragma endregion Response Object

#pragma region InGameObject

InGameObject::InGameObject(
	const json& objectJson,
	ObjectType objectType,
	const int32_t classId,
	const int32_t globalId )
		: m_ObjectType( objectType )
{
	m_GlobalId = globalId;

	// validate the objectID == array index strategy
	m_ClassId = objectJson.at( "Id" );
	assert( m_ClassId == classId );

	m_Name = objectJson.at( "Name" );

	// Locations and items have descriptions
	m_Description = objectJson.value( "Description", "" );

	if( objectJson.contains( "Location" ) )
	{
		m_Location = objectJson.at( "Location" );
		m_DefaultLocation = m_Location;
	}
	else
	{
		// Set to INVALID
		m_Location = INVALID;
		m_DefaultLocation = INVALID;
	}

	assert( objectJson.at( "Responses" ).is_array() );
	for( auto& it : objectJson.at( "Responses" ) )
	{
		ResponseType type = ResponseType::RESPONSE_TYPE_INVALID;
		std::string jsonType = it.at( "Type" );

		if( "Take" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_TAKE;
		}
		else if( "Examine" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_EXAMINE;
		}
		else if( "Discard" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_DISCARD;
		}
		else if( "Throw" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_THROW;
		}
		else if( "Interaction" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_INTERACTION;
		}
		else if( "Attack" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_ATTACK;
		}
		else if( "Transact" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_TRANSACT;
		}
		else
		{
			continue;
		}

		const std::shared_ptr<Response>& responsePtr = m_Responses[ static_cast< size_t >( type ) ].emplace_back( std::make_shared<Response>() );
		Response* response = responsePtr.get();

		response->SetRequiresPossession( it.value( "RequiresPossession", false ) );
		response->SetRequiredLocation( it.value( "RequiresLocation", INVALID ) );

		if( it.contains( "RequiresVerb" ) )
		{
			response->SetRequiredVerb( it.at( "RequiresVerb" ) );
		}

		if( it.contains( "RequiresEvent" ) )
		{
			response->SetRequiredEvent( it.at( "RequiresEvent" ) );
		}

		if( it.contains( "TriggersEvent" ) )
		{
			response->SetTriggeredEvent( it.at( "TriggersEvent" ) );
		}

		// Not all responses have text, but if it's present it might be an array
		if( it.contains( "Text" ) )
		{
			if( it.at( "Text" ).is_array() )
			{
				for( auto &entry : it.at( "Text" ) )
				{
					response->PushResponseText( entry );
				}
			}
			else
			{
				response->PushResponseText( it.at( "Text" ) );
			}
		}

		// Not all responses have required objects, but if it's present it might be an array
		if( it.contains( "RequiresObject" ) )
		{
			if( it.at( "RequiresObject" ).is_array() )
			{
				for( auto &entry : it.at( "RequiresObject" ) )
				{
					response->PushRequiredObject( entry );
				}
			}
			else
			{
				response->PushRequiredObject( it.at( "RequiresObject" ) );
			}
		}
	}
}

bool InGameObject::IsInvalid( const json& objectJson ) const
{
	return( objectJson.at( "Id" ) == 0 && objectJson.at( "Name" ) == "invalid" );
}

ObjectType InGameObject::GetType( void ) const
{
	return m_ObjectType;
}

int32_t InGameObject::GetObjectClassId( void ) const
{
	return m_ClassId;
}

int32_t InGameObject::GetObjectId( void ) const
{
	return m_GlobalId;
}

int32_t InGameObject::GetLocation( void ) const
{
	return m_Location;
}

int32_t InGameObject::GetDefaultLocation( void ) const
{
	return m_DefaultLocation;
}

void InGameObject::SetLocation( int32_t location )
{
	m_Location = location;
}

const std::string& InGameObject::GetName( void ) const
{
	return m_Name;
}

const std::string& InGameObject::GetDescription( void ) const
{
	return m_Description;
}

std::vector<std::shared_ptr<Response>>& InGameObject::GetResponses( ResponseType type )
{
	assert( ResponseType::RESPONSE_TYPE_INVALID < type && type < ResponseType::RESPONSE_TYPE_MAX );
	return m_Responses[ static_cast<size_t>( type ) ];
}
