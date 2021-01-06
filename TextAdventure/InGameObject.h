#pragma once
#include "stdafx.h"

enum class ResponseType
{
	RESPONSE_TYPE_INVALID = -1,
	RESPONSE_TYPE_TAKE,
	RESPONSE_TYPE_EXAMINE,
	RESPONSE_TYPE_DISCARD,
	RESPONSE_TYPE_THROW,
	RESPONSE_TYPE_INTERACTION,
	RESPONSE_TYPE_ATTACK,
	RESPONSE_TYPE_TRANSACT,
	RESPONSE_TYPE_MAX
};

class InGameObject;

class Response
{
	friend class InGameObject;

public:
	Response( void );

	bool ObjectPossessionIsRequired( void ) const;

	int32_t GetRequiredLocation( void ) const;
	const std::list<int32_t>& GetRequiredObjects( void ) const;

	const std::string& GetRequiredVerb( void ) const;
	const std::string& GetRequiredEvent( void ) const;
	const std::string& GetTriggeredEvent( void ) const;
	std::string GetResponseText( void );

protected:
	void SetRequiresPossession( bool required );
	void PushRequiredObject( int32_t id );
	void SetRequiredLocation( int32_t id );
	void SetRequiredVerb( const std::string &verb );
	void SetRequiredEvent( const std::string &event );
	void SetTriggeredEvent( const std::string &event );
	void PushResponseText( const std::string &text );

private:
	// Response requires the object to be in inventory
	bool		m_RequiresPossession;

	// Response requires character and/or item to be in inventory
	int32_t		m_RequiresLocation;
	std::list<int32_t> m_RequiresObjects;

	// Response requires the use of a specific verb
	std::string m_RequiresVerb;

	// Response requires that a specific event has happened
	std::string m_RequiresEvent;

	// Response triggers an event
	std::string m_TriggersEvent;

	// List of all possible textual outputs from this response
	size_t					 m_CurrentResponse;
	std::vector<std::string> m_Text;
};

enum class ObjectType
{
	OBJECT_ANY,
	OBJECT_LOCATION,
	OBJECT_CHARACTER,
	OBJECT_ITEM,
};

class InGameObject
{
public:
	static const int32_t INVALID = 0;

	InGameObject( const json &objectJson, ObjectType objectType, const int32_t classId, const int32_t globalId );

	bool IsInvalid( const json &objectJson ) const;

	ObjectType GetType( void ) const;
	int32_t GetObjectClassId( void ) const;
	int32_t GetObjectId( void ) const;

	int32_t GetLocation( void ) const;
	int32_t GetDefaultLocation( void ) const;
	void SetLocation( int32_t location );

	const std::string& GetName( void ) const;
	const std::string& GetDescription( void ) const;
	std::vector<std::shared_ptr<Response>>& GetResponses( ResponseType type );

protected:
	int32_t		m_GlobalId;
	int32_t		m_ClassId;
	ObjectType  m_ObjectType;
	int32_t		m_Location;
	int32_t		m_DefaultLocation;
	std::string m_Name;
	std::string m_Description;

	// Responses
	std::array<std::vector<std::shared_ptr<Response>>, static_cast<size_t>( ResponseType::RESPONSE_TYPE_MAX )> m_Responses;
};
