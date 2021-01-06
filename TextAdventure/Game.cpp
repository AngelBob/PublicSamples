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

    // Place the characters and items into their starting locations
    for( auto& object : m_Objects )
    {
        // The first "character" is the invalid character, and it gets placed into the invalid location
        Location& location = m_Map->GetLocation( object->GetLocation() );
        location.AddObject( object->GetObjectId() );

        // Add the character name and Id to the map
        m_ObjectNameToIdMap.insert( std::make_pair( object->GetName(), object->GetObjectId() ) );
    }

    // After everything is loaded, describe the opening scene
    DescribeScene();
}

void Game::OnUnload( void )
{

}

void Game::OnMove( const ParserT& parser )
{
    // On a move, get the direction from the parser and check with the location to see if the
// direction is valid.  If so, execute, otherwise notify the user they can't go that way.
    const std::string &dir = parser.GetLastObject();
    if( m_Map->OnMove( dir ) )
    {
        // Move was good
        // Posse needs to come with us
        for( int32_t objectId : m_Inventory )
        {
            if( ObjectType::OBJECT_CHARACTER == m_Objects.at( objectId )->GetType() )
            {
                m_Objects.at( objectId )->SetLocation( m_Map->GetLocation().GetObjectId() );
            }
        }

        // Describe the new scene
        std::cout << "\n";
        DescribeScene();
    }
    else
    {
        MoveDirection dirEnum = m_Map->GetDirectionEnum( dir );
        if( MoveDirection::MOVE_INVALID == dirEnum )
        {
            std::cout << "I don't know how to go in that direction";
        }
        else
        {
            const std::string &directionName = m_Map->GetDirectionName( dirEnum );
            std::cout << "You are unable to move to the " << directionName;
        }
        std::cout << std::endl;
    }
}

void Game::OnTake( const GameObjectData& objectData, Response* response )
{
    // Player wants to add an object to the inventory
    // Options:
    // 1) It's not even an object, let the user know
    // 2) It is an object
    //    a) it's not here,
    //    b) it's not takeable
    //    c) it's takeable

    if( InGameObject::INVALID != objectData.id )
    {
        // It is an object, let see if it can be added to the inventory
        if( response )
        {
            // It's here and available, add the item to the inventory and remove it from the location
            int32_t itemId = objectData.id;
            m_Inventory.emplace_back( itemId );
            m_Map->GetLocation( m_Objects.at( itemId )->GetLocation() ).RemoveObject( itemId );
            m_Objects.at( itemId )->SetLocation( InGameObject::INVALID );

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
        DescribeScene();
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
            const std::string &name = m_Objects.at( objectData.id ).get()->GetName();
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

void Game::OnThrow( const GameObjectData &objectData, Response *response )
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
            const std::string &name = m_Objects.at( objectData.id ).get()->GetName();
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
    // Get the character(s) from the current location.
    const std::list<int32_t> &objectIds = m_Map->GetLocation().GetObjects();

    // Need to filter the characters from the items at the current location
    std::list<int32_t> characterIds;
    std::copy_if( objectIds.begin(),
                  objectIds.end(),
                  std::back_inserter( characterIds ),
                  [ this ]( const int id ) { return ( ObjectType::OBJECT_CHARACTER == m_Objects.at( id )->GetType() ); } );


    if( 0 < characterIds.size() )
    {
        // The intereseting bit... have the character respond (assuming the user is
        // wanting to talk to someone that's actually present).

        // Options:
        // 1) Player provided a character name, and the character is present.
        // 2) Player provided no name.
        //    a) if there is a single character present, do the interaction.
        //    b) if there is more than one character present, tell the user.
        // 3) The character is not present.

        // Default to checking for case 1
        bool doInteraction = objectData.isHere && ( response != nullptr );
        if( !doInteraction )
        {
            // Case 2 or 3
            if( InGameObject::INVALID == objectData.id )
            {
                // This is case 2 - no name provided.
                // How many characters?
                if( characterIds.size() == 1 )
                {
                    // Scenario 2a - single character present
                    doInteraction = true;
                    GameObjectData charData;
                    GetObjectData( m_Objects.at( characterIds.front() ).get()->GetName(), charData );
                    assert( InGameObject::INVALID != charData.id && ObjectType::OBJECT_CHARACTER == charData.type );
                    response = GetBestResponse( charData, "talk", ResponseType::RESPONSE_TYPE_INTERACTION );
                }
                else
                {
                    // No name provided, and multiple characters are present; this is scenario 2b.
                    std::cout << "Are you talking to ";
                    auto start = characterIds.begin();
                    auto stop = characterIds.end();
                    --stop;

                    while( start != stop )
                    {
                        std::cout << m_Objects.at( *start )->GetName() << ", ";
                        ++start;
                    }
                    std::cout << "or " << m_Objects.at( *start )->GetName() << "?" << std::endl;
                }
            }
            else
            {
                // This is case 3 - name was provided, but character is not present
                if( ObjectType::OBJECT_CHARACTER == objectData.type )
                {
                    std::cout << m_Objects.at( objectData.id ).get()->GetName() << " isn't here." << std::endl;
                }
                else
                {
                    std::cout << "You probably shouldn't try talking to the " << m_Objects.at( objectData.id ).get()->GetName() << std::endl;
                }
            }
        }

        if( doInteraction )
        {
            std::cout << response->GetResponseText() << std::endl;
        }
    }
    else
    {
        // There are no characters here, the user is crazy.
        std::cout << "There's no one here, are you responding to the voices in your head?" << std::endl;
    }
}

void Game::OnAttack( const GameObjectData &objectData, Response *response )
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

void Game::OnTransact( const GameObjectData &objectData, Response *response )
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
        std::cout << "You have " << m_Objects.at( items.front() )->GetName() << " in your pocket.\n";
    }
    else
    {
        std::cout << "You have " << items.size() << " item in your pockets:\n";

        auto start = items.begin();
        auto stop = items.end();
        --stop;

        while( start != stop )
        {
            std::cout << m_Objects.at( *start )->GetName() << "\n";
            ++start;
        }

        std::cout << "and\n";
        std::cout << m_Objects.at( *start )->GetName() << std::endl;
    }
}

void Game::OnTrigger( Response *response )
{
    if( response != nullptr && !response->GetTriggeredEvent().empty() )
    {
        // Do some stuff with the event
        //std::cout << response->GetTriggeredEvent() << std::endl;
    }
}

//private:
void Game::DescribeScene( void )
{
    std::stringstream ss;

    Location &curLocation = m_Map->GetLocation();
    ss << curLocation.GetName();
    ss << "\n";

    if( curLocation.GetShownOnce() )
    {
        ss << curLocation.GetDescription();
    }
    else
    {
        ss << curLocation.GetResponses( ResponseType::RESPONSE_TYPE_EXAMINE ).front().get()->GetResponseText();
    }

    size_t numNeighbors;
    PrintDirectionsAsSeen( ss, numNeighbors );
    if( 0 != numNeighbors )
    {
        ss << "\n";
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

void Game::LoadGameResources()
{
    // Need to load up the character and item resources
    static const ResourceList resources[ 2 ] = {
        { L"CHARACTERS", IDR_CHARACTERS1 },
        { L"ITEMS", IDR_ITEMS1 },
    };

    for( int32_t idx = 0; idx < _countof( resources); ++idx )
    {
        ObjectType objType;
        json items;

        ResourceLoader::LoadStringResource( items, resources[ idx ].name, resources[ idx ].id );
        switch( resources[ idx ].id )
        {
        case IDR_CHARACTERS1:
            objType = ObjectType::OBJECT_CHARACTER;
            break;
        case IDR_ITEMS1:
            objType = ObjectType::OBJECT_ITEM;
            break;
        }
        ResourceLoader::BuildObjects<InGameObject>( items, objType, m_Objects );
    }
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
            os << m_Objects.at( *cur )->GetName();
            if( 2 != characterIds.size() )
            {
                os << ", ";
            }
            ++cur;
        }

        if( 2 == characterIds.size() )
        {
            os << " and ";
        }
        os << m_Objects.at( *cur )->GetName();

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

void Game::GetObjectData( const std::string& name, GameObjectData& objectData ) const
{
    // Get the object ID for the item "name"
    objectData.id = InGameObject::INVALID;
    auto objectIt = m_ObjectNameToIdMap.find( name );
    if( objectIt != m_ObjectNameToIdMap.end() )
    {
        objectData.id = objectIt->second;
    }

    // Get the item's current location
    if( InGameObject::INVALID != objectData.id )
    {
        // Get the object type
        objectData.type = m_Objects.at( objectData.id )->GetType();

        // Get the item's current location
        objectData.locationId = m_Objects.at( objectData.id )->GetLocation();

        // Is the item in the inventory?
        auto invIt = std::find( m_Inventory.begin(), m_Inventory.end(), objectData.id );
        objectData.haveIt = ( invIt != m_Inventory.end() );

        // Is the object in the same location as the player?
        // Either in the inventory or at the same map location.
        objectData.isHere = objectData.haveIt || ( objectData.locationId == m_Map->GetLocation().GetObjectId() );
    }
    else
    {
        objectData.locationId = InGameObject::INVALID;
        objectData.haveIt = false;
        objectData.isHere = false;
    }
}

Response* Game::GetBestResponse( const GameObjectData &objectData, const std::string &verb, const ResponseType& type) const
{
    // Find the best available response object
    std::vector<std::shared_ptr<Response>>& responses = m_Objects.at( objectData.id )->GetResponses( type );

    std::vector<Response*> validResponses;
    if( 0 == responses.size() )
    {
        validResponses.emplace_back( nullptr );
    }
    else if( !objectData.isHere )
    {
        // To interact, the item and the user must be in the same location
        validResponses.emplace_back( nullptr );
    }
    else if( 1 == responses.size() )
    {
        validResponses.emplace_back( responses.front().get() );
    }
    else
    {
        // The interesting case.  Need to find responses for which all necessary criteria are met
        for( auto &it : responses )
        {
            Response *thisResponse = it.get();

            if( thisResponse->ObjectPossessionIsRequired() && !objectData.haveIt )
            {
                // Object must be in inventory, but it's not.
                // This response is not valid at this time.
                continue;
            }

            if(    thisResponse->GetRequiredLocation() != InGameObject::INVALID
                && thisResponse->GetRequiredLocation() != m_Map->GetLocation().GetObjectId() )
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

            const std::list<int32_t>& requiredObjects = thisResponse->GetRequiredObjects();
            if( 0 < requiredObjects.size() )
            {
                bool criteriaMet = true;
                for( int32_t id : requiredObjects )
                {
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

            if( 0 < m_TriggeredEvents.size() )
            {
                bool criteriaMet = true;
                for( const std::string& event : m_TriggeredEvents )
                {
                    if( m_TriggeredEvents.end() == std::find( m_TriggeredEvents.begin(), m_TriggeredEvents.end(), thisResponse->GetRequiredEvent() ) )
                    {
                        // Object requires an event to have happened, but it's hasn't.
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

            // If we're here, the response is valid.  Push it into the valid responses list
            validResponses.emplace_back( thisResponse );
        }
    }

    Response *response = nullptr;
    if( 1 == validResponses.size() )
    {
        response = validResponses.front();
    }
    else
    {
        // More than one, find the most appropriate.
        // Preferred order:
        // 1) responses that trigger an event,
        // 2) responses that require an event,
        // 3) responses that have no requirements
        // The response requirements should be set such that there can
        // be at most one of each of the above catagories present
        for( auto& it : validResponses )
        {
            const std::string& trigger = it->GetTriggeredEvent();
            if( !trigger.empty() )
            {
                // Responses that trigger events are highest priority, no need to keep looking.
                response = it;
                break;
            }
            
            const std::string& event = it->GetRequiredEvent();
            if( nullptr == response && !event.empty() )
            {
                // Responses that require events are next priority, but need to keep looking.
                response = it;
            }

            if( nullptr == response )
            {
                // Vanilla resonses are lowest priority.
                response = it;
            }
        }
    }

    return response;
}

int32_t Game::DoDrop( const GameObjectData &objectData )
{
    // Move the item from inventory to the current location
    int32_t itemId = objectData.id;

    std::list<int32_t>::const_iterator invIter = std::find( m_Inventory.begin(), m_Inventory.end(), itemId );
    assert( invIter != m_Inventory.end() );

    m_Inventory.erase( invIter );

    int32_t newObjectLoc = InGameObject::INVALID;
    if( ObjectType::OBJECT_ITEM == objectData.type )
    {
        // Items stay where they are dropped (the id of the location)
        newObjectLoc = m_Map->GetLocation().GetObjectId();
    }
    else
    {
        // Characters return to their default locations
        newObjectLoc = m_Objects.at( itemId )->GetDefaultLocation();
    }
    m_Map->GetLocation( newObjectLoc ).AddObject( itemId );
    m_Objects.at( itemId )->SetLocation( newObjectLoc );

    return newObjectLoc;
}
