#pragma once
#include "stdafx.h"

enum class ResponseType
{
	RESPONSE_TYPE_INVALID = -1,
	RESPONSE_TYPE_MOVE,
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

	const std::string& GetRequiredLocation( void ) const;
	const std::list<std::string>& GetRequiredObjects( void ) const;
	const std::string& GetRequiredIndirectObject( void ) const;

	const std::string& GetRequiredVerb( void ) const;
	const std::string& GetRequiredEvent( void ) const;
	const std::string& GetTriggeredEvent( void ) const;
	const std::string& GetResponseText( bool doAdvance = true );
	const std::string& GetMoveDirection( void ) const;
	const std::string& GetMoveDestination( void ) const;

protected:
	void SetRequiresPossession( bool required );
	void PushRequiredObject( const std::string& obj );
	void SetRequiredIndirectObject( const std::string& obj );
	void SetRequiredLocation( const std::string& loc );
	void SetRequiredVerb( const std::string& verb );
	void SetRequiredEvent( const std::string& event );
	void SetTriggeredEvent( const std::string& event );
	void PushResponseText( const std::string& text );
	void SetMoveDirection( const std::string& dir );
	void SetMoveDestination( const std::string& dest );

private:
	// Response requires the object to be in inventory
	bool		m_RequiresPossession;

	// Response requires character and/or item to be in inventory
	std::string			   m_RequiresLocation;
	std::list<std::string> m_RequiresObjects;
	std::string			   m_RequiresIndirectObject;

	// Response requires the use of a specific verb
	std::string m_RequiresVerb;

	// Response requires that a specific event has happened
	std::string m_RequiresEvent;

	// Response triggers an event
	std::string m_TriggersEvent;

	// List of all possible textual outputs from this response
	size_t					 m_CurrentResponse;
	std::vector<std::string> m_Text;

	// Movement responses also have a direction and destination
	std::string				 m_Direction;
	std::string              m_DestinationId;
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
	static const int32_t INVALID = -1;

	InGameObject( const json &objectJson, const int32_t globalId );
	virtual ~InGameObject();

	bool IsInvalid( const json &objectJson ) const;

	virtual ObjectType GetType( void ) const = 0;
	int32_t GetObjectId( void ) const;
	const std::string& GetObjectName( void ) const;

	const std::string& GetDisplayName( void ) const;
	const std::string& GetDescription( void ) const;
	const std::vector<std::string>& GetAltNames( void ) const;

	const std::string& GetDefaultLocation( void ) const;

	std::vector<std::shared_ptr<Response>>& GetResponses( ResponseType type );

	bool GetVisibility( void ) const;

protected:
	int32_t		m_GlobalId;
	std::string m_Name;
	std::string m_DisplayName;
	std::string m_Description;
	std::vector<std::string> m_AltNames;

	std::string m_DefaultLocation;

	bool m_IsVisible;

	// Responses
	std::array<std::vector<std::shared_ptr<Response>>, static_cast<size_t>( ResponseType::RESPONSE_TYPE_MAX )> m_Responses;
};

class Character : public InGameObject
{
public:
	inline Character( const json& objectJson, const int32_t globalId )
		: InGameObject( objectJson, globalId )
	{
	}

	inline ObjectType GetType( void ) const
	{
		return ObjectType::OBJECT_CHARACTER;
	}
};

class Item : public InGameObject
{
public:
	inline Item( const json& objectJson, const int32_t globalId )
		: InGameObject( objectJson, globalId )
	{
	}

	inline ObjectType GetType( void ) const
	{
		return ObjectType::OBJECT_ITEM;
	}
};
