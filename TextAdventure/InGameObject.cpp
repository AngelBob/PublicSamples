#include "stdafx.h"
#include "Objects.h"

#pragma region Response Object
Response::Response( void )
	: m_RequiresPossession( false )
	, m_CurrentResponse( 0 )
{
}

bool Response::ObjectPossessionIsRequired( void ) const
{
	return m_RequiresPossession;
}

const std::list<std::string>& Response::GetRequiredObjects( void ) const
{
	return m_RequiresObjects;
}

const std::string& Response::GetRequiredIndirectObject( void ) const
{
	return m_RequiresIndirectObject;
}

const std::string& Response::GetRequiredLocation( void ) const
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

const std::string& Response::GetResponseText( bool doAdvance )
{
	if( 0 < m_Text.size() )
	{
		const std::string &text = m_Text.at( m_CurrentResponse );

		if( doAdvance )
		{
			++m_CurrentResponse;
			if( m_CurrentResponse >= m_Text.size() )
			{
				m_CurrentResponse = m_Text.size() - 1;
			}
		}

		return text;
	}
	else
	{
		static const std::string blank;
		return blank;
	}
}

const std::string &Response::GetMoveDirection( void ) const
{
	return m_Direction;
}

const std::string& Response::GetMoveDestination( void ) const
{
	return m_DestinationId;
}

void Response::SetRequiresPossession( bool required )
{
	m_RequiresPossession = required;
}

void Response::PushRequiredObject( const std::string& obj )
{
	m_RequiresObjects.emplace_back( obj );
}

void Response::SetRequiredIndirectObject( const std::string& obj )
{
	m_RequiresIndirectObject = obj;
}

void Response::SetRequiredLocation( const std::string& loc )
{
	m_RequiresLocation = loc;
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

void Response::SetMoveDirection( const std::string &dir )
{
	m_Direction = dir;
}

void Response::SetMoveDestination( const std::string& dest )
{
	m_DestinationId = dest;
}
#pragma endregion Response Object

#pragma region InGameObject

InGameObject::InGameObject(
	const json& objectJson,
	const int32_t globalId )
	: m_GlobalId( globalId )
{
	// Textual identifications and description for this game object
	m_Name = objectJson.at( "Id" );
	m_DisplayName = objectJson.at( "DisplayName" );
	if( objectJson.contains( "AltNames" ) )
	{
		// Load up the array of alternate names for this object
		for( auto &it : objectJson.at( "AltNames" ) )
		{
			m_AltNames.push_back( it );
		}
	}
	m_Description = objectJson.value( "Description", "" );

	// The default location for this object.
	if( objectJson.contains( "Location" ) )
	{
		m_DefaultLocation = objectJson.at( "Location" );
	}
	m_IsVisible = objectJson.value( "IsVisible", true );

	// Objects can be invisible, but still present
	// These objects require a transaction to acquire
	m_IsPresent = objectJson.value( "IsPresent", false );

	assert( objectJson.at( "Responses" ).is_array() );
	for( auto& it : objectJson.at( "Responses" ) )
	{
		ResponseType type = ResponseType::RESPONSE_TYPE_INVALID;
		std::string jsonType = it.at( "Type" );

		if( "Movement" == jsonType )
		{
			type = ResponseType::RESPONSE_TYPE_MOVE;
		}
		else if( "Take" == jsonType )
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

		if( it.contains( "RequiresLocation" ) )
		{
			response->SetRequiredLocation( it.at( "RequiresLocation" ) );
		}

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

		if( it.contains( "Direction" ) )
		{
			response->SetMoveDirection( it.at( "Direction" ) );
		}

		if( it.contains( "Destination" ) )
		{
			response->SetMoveDestination( it.at( "Destination" ) );
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

		if( it.contains( "RequiresIndirectObject" ) )
		{
			response->SetRequiredIndirectObject( it.at( "RequiresIndirectObject" ) );
		}
	}
}

InGameObject::~InGameObject( void )
{
}

bool InGameObject::IsInvalid( const json& objectJson ) const
{
	return( objectJson.at( "Id" ) == "invalid" );
}

int32_t InGameObject::GetObjectId( void ) const
{
	return m_GlobalId;
}

const std::string& InGameObject::GetObjectName( void ) const
{
	return m_Name;
}

const std::string& InGameObject::GetDisplayName( void ) const
{
	return m_DisplayName;
}

const std::string& InGameObject::GetDescription( void ) const
{
	return m_Description;
}

const std::vector<std::string>& InGameObject::GetAltNames( void ) const
{
	return m_AltNames;
}

const std::string& InGameObject::GetDefaultLocation( void ) const
{
	return m_DefaultLocation;
}

std::vector<std::shared_ptr<Response>>& InGameObject::GetResponses( ResponseType type )
{
	assert( ResponseType::RESPONSE_TYPE_INVALID < type && type < ResponseType::RESPONSE_TYPE_MAX );
	return m_Responses[ static_cast<size_t>( type ) ];
}

bool InGameObject::GetVisibility( void ) const
{
	return m_IsVisible;
}

bool InGameObject::GetPresence(void) const
{
	return m_IsPresent;
}
