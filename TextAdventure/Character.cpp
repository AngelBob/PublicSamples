#include "stdafx.h"
#include "Objects.h"

Character::Character( const json &character )
	: InGameObject( character )
	, m_CurResponse( 0 )
{
	// Bail on invalid object creation
	if( IsInvalid( character ) )
	{
		return;
	}

	// The response list is required
	std::vector<std::string> responses = character.at( "Responses" );
	m_Responses = responses;
	m_CurResponse = 0;
}

std::string &Character::GetResponse( void )
{
	std::string &response = m_Responses[ m_CurResponse ];
	if( m_CurResponse < m_Responses.size() - 1 )
	{
		++m_CurResponse;
	}

	return response;
}
