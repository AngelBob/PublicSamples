#include "stdafx.h"
#include "Objects.h"

InGameEvent::InGameEvent( const json &eventJson, const int32_t globalId )
	: m_GlobalId( globalId )
	, m_Count( 0 )
	, m_MaxCount( 0 )
	, m_IsTriggered( false )
	, m_IsEndGame( false )
{
	m_Name = eventJson.at( "Id" );

	if( eventJson.contains( "Repeats" ) )
	{
		m_MaxCount = eventJson.at( "Repeats" );
	}

	if( eventJson.contains( "Action" ) )
	{
		if( eventJson.at( "Action" ).is_array() )
		{
			for( auto &it : eventJson.at( "Action" ) )
			{
				if( it.contains( "GoTo" ) )
				{
					m_GoToTarget = it.at( "GoTo" );
				}

				if( it.contains( "MakeVisible" ) )
				{
					m_MakeVisibleTarget = it.at( "MakeVisible" );
				}

				if( it.contains( "MakeInvisible" ) )
				{
					m_MakeInvisibleTarget = it.at( "MakeInvisible" );
				}

				if( it.contains( "UntriggerEvent" ) )
				{
					for( auto &event : it.at( "UntriggerEvent" ) )
					{
						m_RewindEvents.emplace_back( event );
					}
				}

				if( it.contains( "EndGame" ) )
				{
					m_IsEndGame = true;
				}
			}
		}
		else
		{
			if( eventJson.at( "Action" ).contains( "GoTo" ) )
			{
				m_GoToTarget = eventJson.at( "Action" ).at( "GoTo" );
			}

			if( eventJson.at( "Action" ).contains( "MakeVisible" ) )
			{
				m_MakeVisibleTarget = eventJson.at( "Action" ).at( "MakeVisible" );
			}

			if( eventJson.at( "Action" ).contains( "MakeInvisible" ) )
			{
				m_MakeInvisibleTarget = eventJson.at( "Action" ).at( "MakeInvisible" );
			}

			if( eventJson.at( "Action" ).contains( "UntriggerEvent" ) )
			{
				for( auto& event : eventJson.at( "Action" ).at( "UntriggerEvent" ) )
				{
					m_RewindEvents.emplace_back( event );
				}
			}

			if( eventJson.at( "Action" ).contains( "EndGame" ) )
			{
				m_IsEndGame = true;
			}
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

void InGameEvent::UnTrigger( void )
{
	m_Count = 0;
	m_IsTriggered = false;
}

bool InGameEvent::IsEndGame( void ) const
{
	return m_IsEndGame;
}

int32_t InGameEvent::GetEventId( void ) const
{
	return m_GlobalId;
}

const std::string& InGameEvent::GetEventName( void ) const
{
	return m_Name;
}

const std::string& InGameEvent::GetEventText( void ) const
{
	return m_Text;
}

const std::string& InGameEvent::GetMakeVisibleTarget( void ) const
{
	return m_MakeVisibleTarget;
}

const std::string& InGameEvent::GetMakeInvisibleTarget( void ) const
{
	return m_MakeInvisibleTarget;
}

const std::string& InGameEvent::GetGoToTarget( void ) const
{
	return m_GoToTarget;
}

const std::vector<std::string>& InGameEvent::GetEventChain( void ) const
{
	return m_EventChain;
}

const std::vector<std::string> &InGameEvent::GetUntriggerChain( void ) const
{
	return m_RewindEvents;
}