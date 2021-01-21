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
			for( auto& it : eventJson.at( "Action" ) )
			{
				LoadAction( it );
			}
		}
		else
		{
			LoadAction( eventJson.at( "Action" ) );
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

const std::map<std::string, std::string>& InGameEvent::GetMoveObjects( void ) const
{
	return m_MoveObjects;
}

const std::vector<std::string>& InGameEvent::GetEventChain( void ) const
{
	return m_EventChain;
}

const std::vector<std::string> &InGameEvent::GetUntriggerChain( void ) const
{
	return m_RewindEvents;
}

//private:
void InGameEvent::LoadAction( const json& action )
{
	if( action.contains( "MoveObject" ) )
	{
		if( action.at( "MoveObject" ).is_array() )
		{
			for( auto& moveObj : action.at( "MoveObject" ) )
			{
				for( auto& [ key, value ] : moveObj.items() )
				{
					m_MoveObjects.insert( std::make_pair( key, value ) );
				}
			}
		}
		else
		{
			for( auto& [ key, value ] : action.at( "MoveObject" ).items() )
			{
				m_MoveObjects.insert( std::make_pair( key, value ) );
			}
		}
	}

	if( action.contains( "MakeVisible" ) )
	{
		m_MakeVisibleTarget = action.at( "MakeVisible" );
	}

	if( action.contains( "MakeInvisible" ) )
	{
		m_MakeInvisibleTarget = action.at( "MakeInvisible" );
	}

	if( action.contains( "UntriggerEvent" ) )
	{
		for( auto &event : action.at( "UntriggerEvent" ) )
		{
			m_RewindEvents.emplace_back( event );
		}
	}

	if( action.contains( "EndGame" ) )
	{
		m_IsEndGame = true;
	}
}