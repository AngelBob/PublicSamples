#pragma once

class Game;

class InGameEvent
{
public:
	InGameEvent( const json &objectJson, const int32_t globalId );

	bool OnTrigger( void );
	bool IsTriggered( void );
	void UnTrigger( void );
	bool IsEndGame( void ) const;

	int32_t GetEventId( void ) const;
	const std::string& GetEventName( void ) const;

	const std::string& GetEventText( void ) const;
	const std::string& GetMakeVisibleTarget( void ) const;
	const std::string& GetMakeInvisibleTarget( void ) const;

	const std::map<std::string, std::string>& GetMoveObjects( void ) const;

	const std::vector<std::string>& GetEventChain( void ) const;
	const std::vector<std::string>& GetUntriggerChain( void ) const;

private:
	void LoadAction( const json &action );

	int32_t		m_GlobalId;
	std::string m_Name;

	int32_t		m_Count;
	int32_t		m_MaxCount;
	bool		m_IsTriggered;
	bool		m_IsEndGame;

	std::string m_Text;
	std::string m_MakeVisibleTarget;
	std::string m_MakeInvisibleTarget;

	std::map<std::string, std::string> m_MoveObjects;

	std::vector<std::string> m_EventChain;
	std::vector<std::string> m_RewindEvents;
};