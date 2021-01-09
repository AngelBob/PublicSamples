#pragma once

class Game;

class InGameEvent
{
public:
	InGameEvent( const json &objectJson, const int32_t globalId );

	bool OnTrigger( void );
	bool IsTriggered( void );

	int32_t GetEventId( void );
	const std::string& GetEventName( void );

	const std::string& GetEventText( void );
	const std::string& GetMakeVisibleTarget( void );
	const std::string& GetGoToTarget( void );

	const std::vector<std::string>& GetEventChain( void );

private:
	int32_t		m_GlobalId;
	std::string m_Name;

	int32_t		m_Count;
	int32_t		m_MaxCount;
	bool		m_IsTriggered;

	std::string m_Text;
	std::string m_MakeVisibleTarget;
	std::string m_GoToTarget;

	std::vector<std::string> m_EventChain;
};