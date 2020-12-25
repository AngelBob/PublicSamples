#include "stdafx.h"

#include "Character.h"

Character::Character( const json &character, std::weak_ptr<Map> map )
	: GameObject( character, map )
	, m_CurResponse( 0 )
{
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
