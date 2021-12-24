#include "stdafx.h"
#include "Objects.h"

const Map& Game::GetMap( void )
{
    return *m_Map;
}

void Game::OnLoad( void )
{
    // Create the map.
    // The map object will create all of the location objects
    m_Map = std::make_unique<Map>();
    m_Map->OnLoad();

    // Create the character and item objects
    LoadGameResources();

    // Do some setup on each object
    for( auto& object : m_Objects )
    {
        // Place the characters and items into their starting locations
        if( object->GetVisibility() )
        {
            // Visible objects get placed in their default locations.
            Location &location = m_Map->GetLocation( object->GetDefaultLocation() );
            location.AddObject( object->GetObjectId() );
        }

        // Add the game object name and id to the map
        m_ObjectNameToIdMap.insert( std::make_pair( object->GetDisplayName(), object->GetObjectId() ) );
        for( auto &altName : object->GetAltNames() )
        {
            m_ObjectNameToIdMap.insert( std::make_pair( altName, object->GetObjectId() ) );
        }

        m_ObjectIdToGlobalIdMap.insert( std::make_pair( object->GetObjectName(), object->GetObjectId() ) );

    }

    // Create the events
    LoadGameEvents();

    // Create the event name to id map
    for( auto& event : m_Events )
    {
        m_EventNameToIdMap.insert( std::make_pair( event->GetEventName(), event->GetEventId() ) );
    }

    // After everything is loaded, describe the opening scene
    DescribeScene();
}

void Game::OnUnload( void )
{

}

void Game::OnMove( const GameObjectData& objectData, Response* response )
{
    // Check the move response to see if it's a valid move, if so, notify the map.
    if( nullptr != response )
    {
        // Move was good
        m_Map->GetLocation().SetShownOnce();
        int32_t oldLoc = m_Map->GetLocation().GetObjectId();

        m_Map->OnMove( response->GetMoveDestination() );

        int32_t newLoc = m_Map->GetLocation().GetObjectId();

        bool needNL = false;
        if( !response->GetResponseText( false ).empty() )
        {
            std::cout << response->GetResponseText() << std::endl;
            needNL = true;
        }

        if( oldLoc != newLoc )
        {
            // Posse needs to come with us
            for( int32_t objectId : m_Inventory )
            {
                if( ObjectType::OBJECT_CHARACTER == m_Objects.at( objectId )->GetType() )
                {
                    m_Map->GetLocation( oldLoc ).RemoveObject( objectId );
                    m_Map->GetLocation( newLoc ).AddObject( objectId );
                }
            }

            // Describe the new scene
            if( needNL )
            {
                std::cout << "\n";
            }
            DescribeScene();
        }
    }
    else
    {
        if( MoveDirection::MOVE_INVALID == objectData.dir )
        {
            std::cout << "I don't know how to go in that direction";
        }
        else
        {
            const std::string &directionName = m_Map->GetDirectionName( objectData.dir );
            std::cout << "You are unable to move to the " << directionName;
        }
        std::cout << std::endl;
    }
}

void Game::OnTake( const GameObjectData& objectData, Response* response )
{
    // Player wants to add an object to the inventory
    if( InGameObject::INVALID != objectData.id )
    {
        // It is an object, let see if it can be added to the inventory
        if( response )
        {
            // It's here and available; remove the item from the location and add it to the inventory.
            int32_t itemId = objectData.id;
            m_Map->GetLocation().RemoveObject( itemId );
            m_Inventory.emplace_back( itemId );

            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            // Item isn't takeable
            std::cout << "That didn't work." << std::endl;
        }
    }
    else
    {
        // Case 1
        std::cout << "I don't know what that is, so you won't be taking it." << std::endl;
    }
}

void Game::OnExamine( const GameObjectData& objectData, Response* response )
{
    // Look more closely at the environment or an item.
    // Some items may need to be in inventory before examination.
    if( InGameObject::INVALID == objectData.id )
    {
        // User wants to look at the environment
        DescribeScene( true );
    }
    else
    {
        // User wants to look at a specific item or character.

        // Figure out what the user wants to look at and what's here.
        // Is it an item or a character?
        if( response )
        {
            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            assert( !"What's happened?" );
        }
    }
}

void Game::OnDiscard( const GameObjectData& objectData, Response* response )
{
    // User wants to drop something from inventory.
    if( InGameObject::INVALID != objectData.id )
    {
        if( response )
        {
            DoDrop( objectData );
            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            const std::string &name = m_Objects.at( objectData.id ).get()->GetDisplayName();
            if( ObjectType::OBJECT_ITEM == objectData.type )
            {
                std::cout << "You don't have the " << name << " in your pockets." << std::endl;
            }
            else if( !objectData.haveIt )
            {
                std::cout << name << " is not with you." << std::endl;
            }
            else
            {
                std::cout << name << " refuses to leave the posse." << std::endl;
            }
        }
    }
    else
    {
        std::cout << "I'm not sure what it is you want to discard." << std::endl;
    }
}

void Game::OnThrow( const GameObjectData& objectData, Response* response )
{
    // User wants to throw something from inventory.
    if( InGameObject::INVALID != objectData.id )
    {
        if( response )
        {
            // Remove the item from inventory and add it to the location
            DoDrop( objectData );
            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            const std::string &name = m_Objects.at( objectData.id ).get()->GetDisplayName();
            if( ObjectType::OBJECT_CHARACTER == objectData.type )
            {
                // Can't throw characters at all.
                std::string filler;
                if( !objectData.isHere )
                {
                    filler = "probably would not";
                }
                else
                {
                    filler = "definitely does not";
                }

                std::cout << "The " << name << " " << filler << " " << "appreciate your attempt." << std::endl;
            }
            else if( !objectData.haveIt )
            {
                // Can't throw something the user doesn't have.
                std::cout << "The " << name << " is not in one of your pockets." << std::endl;
            }
        }
    }
    else
    {
        std::cout << "I'm sorry, I can't allow you to throw that.  It might hurt someone." << std::endl;
    }
}

void Game::OnInteraction( const GameObjectData& objectData, Response* response )
{
    if( response )
    {
        std::cout << response->GetResponseText() << std::endl;
    }
    else if( InGameObject::INVALID != objectData.id )
    {
        // Player specified a character that isn't here
        std::cout << "The " << m_Objects.at( objectData.id ).get()->GetDisplayName() << " isn't here." << std::endl;
    }
    else
    {
        // This only happens when:
        // 1) there are no characters in the location or
        // 2) there is more than one character in the location.

        // Get the character(s) from the current location.
        const std::list<int32_t> &objectIds = m_Map->GetLocation().GetObjects();

        // Need to filter the characters from the items at the current location
        std::list<int32_t> characterIds;
        std::copy_if( objectIds.begin(),
            objectIds.end(),
            std::back_inserter( characterIds ),
            [this]( const int id ) { return ( ObjectType::OBJECT_CHARACTER == m_Objects.at( id )->GetType() ); } );

        if( 0 == characterIds.size() )
        {
            // Case 1: there are no characters present, the player is crazy.
            std::cout << "There's no one here, are you responding to the voices in your head?" << std::endl;
        }
        else
        {
            // Case 2:
            // There's more than one character present, and the player didn't specify which.
            assert( 2 <= characterIds.size() );

            std::cout << "Are you talking to ";
            auto start = characterIds.begin();
            auto stop = characterIds.end();
            --stop;

            while( start != stop )
            {
                std::cout << "the " << m_Objects.at( *start )->GetDisplayName();
                if( 2 > characterIds.size() )
                {
                    std::cout << ", ";
                }
                ++start;
            }
            std::cout << " or the " << m_Objects.at( *start )->GetDisplayName() << "?" << std::endl;
        }
    }
}

void Game::OnAttack( const GameObjectData& objectData, Response* response )
{
    // Player is attacking someone or something.
    if( InGameObject::INVALID != objectData.id )
    {
        if( response )
        {
            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            std::cout << "You should try taking your frustrations out at the gym." << std::endl;
        }
    }
    else
    {
        std::cout << "Are you fighting the visions in your head?" << std::endl;
    }
}

void Game::OnTransact( const GameObjectData& objectData, Response* response )
{
    if( InGameObject::INVALID != objectData.id )
    {
        if( response )
        {
            std::cout << response->GetResponseText() << std::endl;
        }
        else
        {
            std::cout << "I'm really not sure how to do that." << std::endl;
        }
    }
    else
    {
        std::cout << "What a strange person you are." << std::endl;
    }
}

void Game::OnInventory( void ) const
{
    // Need to filter inventory to extract the objects
    std::list<int32_t> items;
    std::copy_if( m_Inventory.begin(),
        m_Inventory.end(),
        std::back_inserter( items ),
        [this]( const int32_t id ) { return ( ObjectType::OBJECT_ITEM == m_Objects.at( id )->GetType() ); } );

    if( 0 == items.size() > 0 )
    {
        std::cout << "Your pockets are empty." << std::endl;
    }
    else if( 1 == items.size() )
    {
        std::cout << "You have " << m_Objects.at( items.front() )->GetDisplayName() << " in your pocket.\n";
    }
    else
    {
        std::cout << "You have " << items.size() << " item in your pockets:\n";

        auto start = items.begin();
        auto stop = items.end();
        --stop;

        while( start != stop )
        {
            std::cout << m_Objects.at( *start )->GetDisplayName() << "\n";
            ++start;
        }

        std::cout << "and\n";
        std::cout << m_Objects.at( *start )->GetDisplayName() << std::endl;
    }
}

void Game::OnTrigger( Response* response )
{
    if( response != nullptr && !response->GetTriggeredEvent().empty() )
    {
        // Look through the event list for the event triggered by this response
        int32_t eventId = m_EventNameToIdMap.at( response->GetTriggeredEvent() );

        InGameEvent* event = m_Events.at( eventId ).get();
        OnTrigger( event, !response->GetResponseText( false ).empty() );
    }
}

void Game::OnTrigger( InGameEvent* event, bool needNL )
{
    // Events should trigger only one time.
    if( !event->IsTriggered() && event->OnTrigger() )
    {
        // Event fires, take action(s)
        if( event->IsEndGame() )
        {
            m_GameOver = true;
        }

        if( !event->GetEventText().empty() )
        {
            std::cout << event->GetEventText() << std::endl;
            needNL = true;
        }

        if( !event->GetMakeVisibleTarget().empty() )
        {
            // Need to make an item visible.  Do that by moving the object to it's default location.
            int32_t objId = m_ObjectIdToGlobalIdMap.at( event->GetMakeVisibleTarget() );
            m_Map->GetLocation( m_Objects.at( objId )->GetDefaultLocation() ).AddObject( objId );
        }

        if( !event->GetMakeInvisibleTarget().empty() )
        {
            // Need to make an item invisible.
            // Item may be in inventory or in it's default location.
            int32_t objId = m_ObjectIdToGlobalIdMap.at( event->GetMakeInvisibleTarget() );
            std::list<int32_t>::const_iterator invIter = std::find( m_Inventory.begin(), m_Inventory.end(), objId );
            if( m_Inventory.end() != invIter )
            {
                // Item is in inventory, just remove it.
                m_Inventory.erase( invIter );
            }
            else
            {
                // Item is in the world, locate it and remove it from it's current location
                m_Map->GetLocation( m_Map->FindObject( objId ) ).RemoveObject( objId );
            }
        }

        for( const std::pair<std::string, std::string>& moveObj : event->GetMoveObjects() )
        {
            // Need to move either a game object or the player
            if( "player" == moveObj.first )
            {
                m_Map->OnMove( moveObj.second );

                // Since the previous response most likely output some text, need to
                // insert an additional new line here to keep the spacing nice.
                if( needNL )
                {
                    std::cout << "\n";
                }
                
                // Player is in a new place, set the scene.
                DescribeScene();
            }
            else
            {
                int32_t objId = m_ObjectIdToGlobalIdMap.at( moveObj.first );

                // Move the object from it's current location to it's target location
                int32_t curLoc = m_Map->FindObject( objId );
                if( InGameObject::INVALID != curLoc )
                {
                    m_Map->GetLocation( curLoc ).RemoveObject( objId );
                }

                if( "invalid" != moveObj.second )
                {
                    m_Map->GetLocation( moveObj.second ).AddObject( objId );
                }
            }
        }

        // Recurse...
        for( auto& chainedName : event->GetEventChain() )
        {
            InGameEvent* chainedEvent = m_Events.at( m_EventNameToIdMap.at( chainedName ) ).get();
            OnTrigger( chainedEvent, needNL );
        }

        // Untrigger...
        for( auto &untriggerName : event->GetUntriggerChain() )
        {
            InGameEvent* untriggerEvent = m_Events.at( m_EventNameToIdMap.at( untriggerName ) ).get();
            untriggerEvent->UnTrigger();
        }
    }
}

//private:
void Game::DescribeScene( bool doFullDescription )
{
    std::stringstream ss;

    Location &curLocation = m_Map->GetLocation();
    ss << curLocation.GetDisplayName();
    ss << "\n";

    if( doFullDescription || !curLocation.GetShownOnce() )
    {
        ss << curLocation.GetResponses( ResponseType::RESPONSE_TYPE_EXAMINE ).front().get()->GetResponseText();

        size_t numNeighbors;
        PrintDirectionsAsSeen( ss, numNeighbors );
        if( 0 != numNeighbors )
        {
            ss << "\n";
        }
    }
    else
    {
        ss << curLocation.GetDescription();
    }

    size_t numCharacters;
    PrintCharacters( ss, numCharacters );
    if( 0 != numCharacters )
    {
        ss << "\n";
    }

    size_t numItems;
    PrintItems( ss, numItems );
    if( 0 != numItems )
    {
        ss << "\n";
    }

    // Remove the final trailing newline, as it will be replaced with std::endl to push the flush
    ss.seekp( -1, std::ios_base::end );
    ss << " ";

    std::string message( ss.str() );
    std::cout << message << std::endl;
}

void Game::LoadGameResources( void )
{
    // Need to load up the character and item resources
    static const ResourceList resources[ 2 ] = {
        { L"CHARACTERS", IDR_CHARACTERS1 },
        { L"ITEMS", IDR_ITEMS1 },
    };

    for( int32_t idx = 0; idx < _countof( resources); ++idx )
    {
        json items;

        ResourceLoader::LoadStringResource( items, resources[ idx ].name, resources[ idx ].id );
        switch( resources[ idx ].id )
        {
        case IDR_CHARACTERS1:
            ResourceLoader::BuildObjects<Character>( items, m_Objects );
            break;
        case IDR_ITEMS1:
            ResourceLoader::BuildObjects<Item>( items, m_Objects );
            break;
        }
    }
}

void Game::LoadGameEvents( void )
{
    json items;
    ResourceLoader::LoadStringResource( items, L"EVENTS", IDR_EVENTS1 );
    ResourceLoader::BuildEvents( items, m_Events );
}

std::ostream &Game::PrintDirectionsAsSeen( std::ostream &os, size_t &numNeighbors ) const
{
    const std::map<MoveDirection, int32_t> neighbors = m_Map->GetLocation().GetNeighbors();
    if( neighbors.size() )
    {
        std::map<MoveDirection, int32_t>::const_iterator cur = neighbors.begin();
        std::map<MoveDirection, int32_t>::const_iterator stop = neighbors.end();

        while( cur != stop )
        {
            if( MoveDirection::MOVE_UP == cur->first )
            {
                os << "  Above you";
            }
            else if( MoveDirection::MOVE_UP == cur->first )
            {
                os << "  Below you";
            }
            else
            {
                os << "  To the " << m_Map->GetDirectionName( cur->first );
            }
            os << " you see " << m_Map->GetLocation( cur->second ).GetAsSeenDescription();

            ++cur;
        }
    }

    numNeighbors = neighbors.size();

    return os;
}

std::ostream& Game::PrintCharacters( std::ostream &os, size_t &numCharacters ) const
{
    const std::list<int32_t>& objectIds = m_Map->GetLocation().GetObjects();

    // Need to filter the characters from the items at the current location
    std::list<int32_t> characterIds;
    std::copy_if( objectIds.begin(),
                  objectIds.end(),
                  std::back_inserter( characterIds ),
                  [ this ]( const int32_t id ) { return ( ObjectType::OBJECT_CHARACTER == m_Objects.at( id )->GetType() ); } );

    if( characterIds.size() )
    {
        std::list<int32_t>::const_iterator cur = characterIds.begin();
        std::list<int32_t>::const_iterator stop = characterIds.end();
        --stop;

        // Character printing starts on a new line, so no space before.
        os << "The ";
        while( cur != stop )
        {
            os << m_Objects.at( *cur )->GetDisplayName();
            if( 2 != characterIds.size() )
            {
                os << ", the ";
            }
            ++cur;
        }

        if( 2 == characterIds.size() )
        {
            os << " and the ";
        }
        os << m_Objects.at( *cur )->GetDisplayName();

        if( characterIds.size() == 1 )
        {
            os << " is";
        }
        else
        {
            os << " are";
        }
        os << " here.";
    }

    numCharacters = characterIds.size();

    return os;
}

std::ostream& Game::PrintItems( std::ostream &os, size_t &numItems ) const
{
    const std::list<int32_t> &objectIds = m_Map->GetLocation().GetObjects();

    // Need to filter the characters from the items at the current location
    std::list<int32_t> itemIds;
    std::copy_if( objectIds.begin(),
        objectIds.end(),
        std::back_inserter( itemIds ),
        [ this ]( const int32_t id ) { return ( ObjectType::OBJECT_ITEM == m_Objects.at( id )->GetType() ); } );

    if( itemIds.size() )
    {
        std::list<int32_t>::const_iterator cur = itemIds.begin();
        std::list<int32_t>::const_iterator stop = itemIds.end();
        --stop;

        // Item printing starts on a new line, so no space before.
        if( itemIds.size() > 1 )
        {
            os << "There are ";
        }
        else
        {
            os << "There is ";
        }

        while( cur != stop )
        {
            os << m_Objects.at( *cur )->GetDescription();
            if( 2 != itemIds.size() )
            {
                os << ", ";
            }
            ++cur;
        }

        if( 2 == itemIds.size() )
        {
            os << " and ";
        }
        os << m_Objects.at( *cur )->GetDescription();

        os << " here.";
    }

    numItems = itemIds.size();

    return os;
}

void Game::GetObjectData( const std::string& name, GameObjectData& objectData, Parser::ParsedType parsedType ) const
{
    // Get the object ID for the item "name"
    objectData.id = InGameObject::INVALID;
    auto objectIt = m_ObjectNameToIdMap.find( name );
    if( objectIt != m_ObjectNameToIdMap.end() )
    {
        objectData.id = objectIt->second;
    }

    // Special case for the "INTERACTION" response type - it's possible the player didn't specify a character name
    // Need to filter the characters from the items at the current location
    if( InGameObject::INVALID == objectData.id && Parser::ParsedType::PARSED_TYPE_INTERACTION == parsedType )
    {
        const std::list<int32_t> &objectIds = m_Map->GetLocation().GetObjects();

        std::list<int32_t> characterIds;
        std::copy_if( objectIds.begin(),
                      objectIds.end(),
                      std::back_inserter( characterIds ),
                      [this]( const int id ) { return ( ObjectType::OBJECT_CHARACTER == m_Objects.at( id )->GetType() ); } );

        if( 1 == characterIds.size() )
        {
            // There's only a single character present in this location, assume talking to that character.
            objectData.id = characterIds.front();
        }
    }

    // Get the item's current location
    if( InGameObject::INVALID != objectData.id )
    {
        // Get the object type
        objectData.type = m_Objects.at( objectData.id )->GetType();

        // Objects don't have movement directions
        objectData.dir = MoveDirection::MOVE_INVALID;

        // Where is the object now?
        objectData.locationId = m_Map->FindObject( objectData.id );

        // Is the item in the inventory?
        auto invIt = std::find( m_Inventory.begin(), m_Inventory.end(), objectData.id );
        objectData.haveIt = ( invIt != m_Inventory.end() );

        // Is the object in the same location as the player?
        // Either in the inventory or at the same map location.
        objectData.isHere = objectData.haveIt || m_Map->GetLocation().HasObject( objectData.id );
    }
    else
    {
        objectData.haveIt = false;
        objectData.isHere = false;
    }
}

void Game::GetMoveData( const std::string& dir, GameObjectData& objectData ) const
{
    // Get the object type
    objectData.type = m_Map->GetLocation().GetType();
    assert( ObjectType::OBJECT_LOCATION == objectData.type );

    // Get the movement direction
    objectData.dir = m_Map->GetDirectionEnum( dir );

    // Get the current location ID; location ID == object ID for locations
    objectData.id = m_Map->GetLocation().GetObjectId();
    objectData.locationId = objectData.id;
    assert( InGameObject::INVALID != objectData.id );

    // Locations can't be in the inventory
    objectData.haveIt = false;

    // Locations are always in the same place as the player
    objectData.isHere = true;
}

Response* Game::GetBestResponse( const GameObjectData& objectData, const std::string& verb, const std::string& indirect, const ResponseType& type) const
{
    // Find the best available response object
    std::vector<std::shared_ptr<Response>>& responses = ( objectData.type != ObjectType::OBJECT_LOCATION ) ? m_Objects.at( objectData.id )->GetResponses( type ) : m_Map->GetLocation().GetResponses( type );

    std::vector<Response*> validResponses;
    // The interesting case.  Need to find responses for which all necessary criteria are met
    for( auto &it : responses )
    {
        Response *thisResponse = it.get();

        if(    ( objectData.type != ObjectType::OBJECT_LOCATION )
            && ( objectData.id   != InGameObject::INVALID )
            && !objectData.isHere )
        {
            // Object isn't here.
            // This response is not valid at this time.
            continue;
        }

        if( thisResponse->ObjectPossessionIsRequired() && !objectData.haveIt )
        {
            // Object must be in inventory, but it's not.
            // This response is not valid at this time.
            continue;
        }

        if(   !thisResponse->GetRequiredLocation().empty()
            && thisResponse->GetRequiredLocation() != m_Map->GetLocation().GetObjectName() )
        {
            // Object must be located in a particular location, but it's not.
            // This response is invalid at this time.
            continue;
        }

        if(   !thisResponse->GetRequiredVerb().empty()
            && thisResponse->GetRequiredVerb() != verb )
        {
            // Response requires a specific verb to have been used, but it wasn't.
            // This response is invalid at this time.
            continue;
        }

        const std::list<std::string>& requiredObjects = thisResponse->GetRequiredObjects();
        if( 0 < requiredObjects.size() )
        {
            bool criteriaMet = true;
            for( const std::string& str : requiredObjects )
            {
                int32_t id = m_ObjectNameToIdMap.at( str );
                if( m_Inventory.end() == std::find( m_Inventory.begin(), m_Inventory.end(), id ) )
                {
                    // Object requires another object to be in inventory, but it's not.
                    // This response is invalid at this time.
                    criteriaMet = false;
                    break;
                }
            }

            if( !criteriaMet )
            {
                continue;
            }
        }

        if(   !thisResponse->GetRequiredIndirectObject().empty()
            && thisResponse->GetRequiredIndirectObject() != indirect )
        {
            // Event requires an indirect object.
            // This response is invalid at this time.
            continue;
        }

        if( !thisResponse->GetRequiredEvent().empty() )
        {
            int32_t eventId = m_EventNameToIdMap.at( thisResponse->GetRequiredEvent() );
            if( !m_Events.at( eventId )->IsTriggered() )
            {
                // Response requires an event that hasn't fired yet.
                // This response is invalid at this time.
                continue;
            }
        }

        // Additional handler for movement responses
        if( ObjectType::OBJECT_LOCATION == objectData.type )
        {
            // Movement requires matching direction as well
            MoveDirection moveDir = m_Map->GetDirectionEnum( thisResponse->GetMoveDirection() );
            if( ( objectData.dir != moveDir ) && ( moveDir != MoveDirection::MOVE_ANY ) )
            {
                // Movement is in the wrong direction for this response
                continue;
            }
        }

        // If we're here, the response is valid.  Push it into the valid responses list
        validResponses.emplace_back( thisResponse );
    }

    // Find the most appropriate response.
    // The most appropriate response is the response that requires an event
    // that has already been triggered.
    Response* response = nullptr;
    for( auto& it : validResponses )
    {
        const std::string& event = it->GetRequiredEvent();
        if( !event.empty() )
        {
            // Check if this required event has been triggered.
            int32_t eventId = m_EventNameToIdMap.at( event );
            if( m_Events.at( eventId )->IsTriggered() )
            {
                if( response )
                {
                    // Check this event ID against the previous event ID.
                    // Keep the larger of the two.
                    const std::string& oldEvent = response->GetRequiredEvent();
                    if( !oldEvent.empty() )
                    {
                        int32_t oldId = m_EventNameToIdMap.at( oldEvent );
                        if( eventId > oldId )
                        {
                            response = it;
                        }
                    }
                    else
                    {
                        // Previous response required no event, so this response
                        // has higher priority.
                        response = it;
                    }
                }
                else
                {
                    response = it;
                }
            }
        }
        else if( nullptr == response )
        {
            // Vanilla responses are always valid in the absense of required event responses.
            response = it;
        }
    }

    return response;
}

void Game::DoDrop( const GameObjectData& objectData )
{
    // Move the item from inventory to the current location
    int32_t itemId = objectData.id;

    std::list<int32_t>::const_iterator invIter = std::find( m_Inventory.begin(), m_Inventory.end(), itemId );

    if( invIter != m_Inventory.end() )
    {
        m_Inventory.erase( invIter );

        if( ObjectType::OBJECT_ITEM == objectData.type )
        {
            // Items stay where they are dropped (the id of the location)
            m_Map->GetLocation().AddObject( itemId );
        }
        else
        {
            // Characters return to their default locations
            m_Map->GetLocation().RemoveObject( itemId );
            m_Map->GetLocation( m_Objects.at( itemId )->GetDefaultLocation() ).AddObject( itemId );
        }
    }
}
