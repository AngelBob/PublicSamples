#include "stdafx.h"
#include "Objects.h"

InGameEvent::InGameEvent( const json &eventJson, const int32_t globalId )
	: m_GlobalId( globalId )
	, m_Count( 0 )
	, m_MaxCount( 0 )
	, m_IsTriggered( false )
{
	m_Name = eventJson.at( "Id" );

	if( eventJson.contains( "Repeats" ) )
	{
		m_MaxCount = eventJson.at( "Repeats" );
	}

	if( eventJson.contains( "Action" ) )
	{
		if( eventJson.at( "Action" ).contains( "GoTo" ) )
		{
			m_GoToTarget = eventJson.at( "Action" ).at( "GoTo" );
		}

		if( eventJson.at( "Action" ).contains( "MakeVisible" ) )
		{
			m_MakeVisibleTarget = eventJson.at( "Action" ).at( "MakeVisible" );
		}
	}

	if( eventJson.contains( "Text" ) )
	{
		m_Text = eventJson.at( "Text" );
	}

	if( eventJson.contains( "TriggersEvent" ) )
	{
		m_EventChain.push_back( eventJson.at( "TriggersEvent" ) );
	}
}

bool InGameEvent::OnTrigger( void )
{
	++m_Count;
	m_IsTriggered = ( m_Count >= m_MaxCount );

	return m_IsTriggered;
}

bool InGameEvent::IsTriggered( void )
{
	return m_IsTriggered;
}

int32_t InGameEvent::GetEventId( void )
{
	return m_GlobalId;
}

const std::string& InGameEvent::GetEventName( void )
{
	return m_Name;
}

const std::string& InGameEvent::GetEventText( void )
{
	return m_Text;
}

const std::string& InGameEvent::GetMakeVisibleTarget( void )
{
	return m_MakeVisibleTarget;
}

const std::string& InGameEvent::GetGoToTarget( void )
{
	return m_GoToTarget;
}

const std::vector<std::string>& InGameEvent::GetEventChain( void )
{
	return m_EventChain;
}