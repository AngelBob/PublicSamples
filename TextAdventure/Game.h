#pragma once

enum class ObjectType;

enum class ResponseType;
class Response;

class Game
{
	// The main game object.
	// This object contains the pointers to all of the InGameObject objects
	// as well as the map object.
	struct ResourceList
	{
		const wchar_t* name;
		const int32_t  id;
	};

public:
	struct GameObjectData
	{
		ObjectType	type;
		MoveDirection dir;
		int32_t		id;
		int32_t		locationId;
		bool		haveIt;
		bool		isHere;
	};

	// Let the compiler generate the default ctor and dtor
	// until there is a compelling reason not to.

	// Get interesting information prior to calling the event handlers.
	void GetObjectData( const std::string &objectName, GameObjectData& objectData ) const;
	void GetMoveData( const std::string &objectName, GameObjectData& objectData ) const;
	Response*GetBestResponse( const GameObjectData& objectData, const std::string &verb, const ResponseType &type ) const;

	// Event handlers
	void OnLoad( void );
	void OnUnload( void );

	void OnMove( const GameObjectData& objectData, Response* response );
	void OnTake( const GameObjectData& objectData, Response* response );
	void OnExamine( const GameObjectData& objectData, Response* response );
	void OnDiscard( const GameObjectData& objectData, Response* response );
	void OnThrow( const GameObjectData& objectData, Response* response );
	void OnInteraction( const GameObjectData& objectData, Response* response );
	void OnAttack(const GameObjectData& objectData, Response* response );
	void OnTransact( const GameObjectData& objectData, Response *response );

	void OnInventory( void ) const;

	void OnTrigger( Response* response );
	void OnTrigger( InGameEvent* event );

	// Accessors
	const Map& GetMap( void );

private:
	void LoadGameResources( void );
	void LoadGameEvents( void );

	// Utility
	void DescribeScene( bool doFullDescription = false );
	std::ostream &PrintDirectionsAsSeen( std::ostream &os, size_t &numNeighbors ) const;
	std::ostream &PrintCharacters( std::ostream &os, size_t &numCharacters ) const;
	std::ostream &PrintItems( std::ostream &os, size_t &numItems ) const;

	int32_t DoDrop( const GameObjectData& objectData );

	// Map contains a list of locations
	std::unique_ptr<Map>	m_Map;

	// Objects are stand-alone things, but relate to the locations
	std::map<std::string, int32_t, typename StringCompareT> m_ObjectIdToGlobalIdMap;
	std::map<std::string, int32_t, typename StringCompareT> m_ObjectNameToIdMap;
	std::vector<std::shared_ptr<InGameObject>> m_Objects;

	// The user inventory
	std::list<int32_t> m_Inventory; // It's possible to collect items and characters

	// Game events
	std::vector<std::shared_ptr<InGameEvent>> m_Events;
	std::map<std::string, int32_t, typename StringCompareT> m_EventNameToIdMap;
};