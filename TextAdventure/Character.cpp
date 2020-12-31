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

	// Build the response lists
	auto& responses = character.at( "Responses" );
	for( auto it = responses.begin(); it != responses.end(); ++it )
	{
		if( it->at( "Type" ) == "Interaction" )
		{
			// These are all of the things a character will say when spoken to.
			auto &sayings = it->at( "Text" );
			for( auto says = sayings.begin(); says != sayings.end(); ++says )
			{
				m_InteractionResponses.emplace_back( *says );
			}
		}
	}

	m_CurResponse = 0;
}

std::ostream& Character::OnInteraction( std::ostream& os )
{
	os << m_InteractionResponses[ m_CurResponse++ ];
	if( m_CurResponse >= m_InteractionResponses.size() )
	{
		--m_CurResponse;
	}

	return os;
}
