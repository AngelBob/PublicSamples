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
    // Cycle through all of the characters fisrt.
    for( auto& character : m_Characters )
    {
        // The first "character" is the invalid character, and it gets placed into the invalid location
        Location &location = m_Map->GetLocation( character->GetLocation() );
        location.AddCharacter( character->GetObjectId() );

        // Add the character name and Id to the map
        m_CharacterNameToIdMap.insert( std::make_pair( character->GetName(), character->GetObjectId() ) );
    }

    // Cycle through all of the items next.
    for( auto &item : m_Items )
    {
        // The first "item" is the invalid item, and it gets placed into the invalid location.
        Location &location = m_Map->GetLocation( item->GetLocation() );
        location.AddItem( item->GetObjectId() );

        // Add the item name and Id to the map
        m_ItemNameToIdMap.insert( std::make_pair( item->GetName(), item->GetObjectId() ) );
    }
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
        for( int32_t characterId : m_Posse )
        {
            m_Characters.at( characterId )->SetLocation( m_Map->GetLocation().GetObjectId() );
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

void Game::OnExamine( const ParserT& parser )
{
    // Look more closely at the environment or an item.
    // Some items may need to be in inventory before examination.
    const std::string &name = parser.GetLastObject();
    if( name.empty() )
    {
        // User wants to look at the environment
        DescribeScene();
    }
    else
    {
        // User wants to look at a specific item or character.

        // Three cases:
        // 1) the item doesn't exist anywhere
        // 2) the item is present in inventory, just do it.
        // 3) the item is present in the environment
        //    a) if the item can be examined in the environment, do it
        //    b) if the item must be in inventory say so.

        // Figure out what the user wants to look at and what's here.
        // Is it an item or a character?
        int32_t itemId;
        int32_t itemLocId;
        bool isInInventory;
        bool isItem = GetItemData( name, itemId, itemLocId, isInInventory );

        // Is it an character?
        int32_t characterId;
        int32_t charLocId;
        bool isInPosse;
        bool isCharacter = GetCharacterData( name, characterId, charLocId, isInPosse );

        if( !isItem && !isCharacter )
        {
            // Case 1) the item/character doesn't exist
            std::cout << "I don't see the " << name << " here." << std::endl;
        }
        else if( isItem )
        {
            // Case 2) the item is present in the user's inventory
            DoItemExamination( name, itemId, isInInventory );
        }
        else
        {
            DoCharacterExamination( name, characterId, isInPosse );
        }
    }
}

void Game::OnTake( const ParserT &parser )
{
    // Player wants to add an item to the inventory
    const std::string &name = parser.GetLastObject();

    // Figure out what the user wants to add and what's here.
    // Is it even an item?
    int32_t itemId;
    int32_t itemLocId;
    bool isInInventory;
    bool isItem = GetItemData( name, itemId, itemLocId, isInInventory );

    // Options:
    // 1) It's not even an object, let the user know
    // 2) It is an object
    //    a) it's not here,
    //    b) it's not takeable
    //    c) it's takeable

    if( !isItem )
    {
        // Case 1
        std::cout << "I don't know what " << name << " is, so you won't be taking that." << std::endl;
    }
    else
    {
        // It is an object, let see if it can be added to the inventory
        bool isHere = ( m_Map->GetLocation().GetObjectId() == itemLocId );
        bool isTakeable = m_Items.at( itemId )->IsTakeable();
        if( isHere && isTakeable )
        {
            // It's here and available, add the item to the inventory and remove it from the location
            m_Inventory.emplace_back( itemId );
            m_Map->GetLocation( m_Items.at( itemId )->GetLocation() ).RemoveItem( itemId );

            std::cout << name << " has been added to your inventory." << std::endl;
        }
        else if( !isHere )
        {
            // Item isn't here
            std::cout << "I don't see the " << name << " here." << std::endl;
        }
        else
        {
            // Item isn't takeable
            std::cout << "You can't take the " << name << " it's much to heavy for you." << std::endl;
        }
    }
}

void Game::OnInteraction( const ParserT &parser )
{
    // Get the character(s) from the current location.
    // If there is more than one, need to tell the user to be more specific.
    const std::list<int32_t> &characterIds = m_Map->GetLocation().GetCharacters();
    if( 0 == characterIds.size() )
    {
        // There are no characters here, the user is crazy.
        std::cout << "There's no one here, are you responding to the voices in your head?" << std::endl;
    }
    else
    {
        // The intereseting bit.. have the character respond (assuming the user is
        // wanting to talk to someone that's actually present).

        // Figure out who the user want to talk to and who's here.
        const std::string &name = parser.GetLastObject();
        int32_t characterId;
        int32_t characterLocId;
        bool isInPosse;
        bool isCharacter = GetCharacterData( name, characterId, characterLocId, isInPosse );

        // Options:
        // 1) only a single character present:
        //    a) if there is a character name, validate that character is present.
        //    b) if the user provided no character name interact with the single character,
        // 2) multiple characters present:
        //    a) if there is a character name, validate that character is present.
        //    b) if the user provided no name, indicate a name is required.
        bool doInteraction = false;
        if( !name.empty() )
        {
            // User provided a name, this is scenarios 1a and 2a
            if(    isInPosse
                || characterLocId == m_Map->GetLocation().GetObjectId() )
            {
                // Requested character is present
                doInteraction = true;
            }
            else
            {
                // Requested character isn't here
                std::cout << name << " isn't here." << std::endl;
            }
        }
        else if( 1 == characterIds.size() )
        {
            // No name provided, but only a single character present; this is scenario 1b.
            // Validate the lists are in sync...
            assert( characterId == InGameObject::INVALID_OBJECT
                || characterId == characterIds.front() );
            characterId = characterIds.front();
            doInteraction = true;
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
                std::cout << m_Characters.at( *start )->GetName() << ", ";
                ++start;
            }
            std::cout << "or " << m_Characters.at( *start )->GetName() << "?" << std::endl;
        }

        if( doInteraction )
        {
            m_Characters.at( characterId )->OnInteraction( std::cout ) << std::endl;
        }
    }
}

void Game::OnInventory( void ) const
{
    if( m_Inventory.size() > 0 )
    {
        std::cout << "You have:\n";

        auto start = m_Inventory.begin();
        auto stop = m_Inventory.end();
        --stop;

        while( start != stop )
        {
            std::cout << m_Items.at( *start )->GetName() << "\n";
            ++start;
        }

        if( m_Inventory.size() > 1 )
        {
            std::cout << "and\n";
        }
        std::cout << m_Items.at( *start )->GetName() << "\nin your pockets" << std::endl;
    }
    else
    {
        std::cout << "Your pockets are empty." << std::endl;
    }
}
void Game::DescribeScene( void )
{
    std::stringstream ss;

    const Location& curLocation = m_Map->GetLocation();
    ss << curLocation.GetName();
    ss << "\n";

    if( curLocation.GetShownOnce() )
    {
        ss << curLocation.GetDescription();
    }
    else
    {
        ss << curLocation.GetExaminationResponse();
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

//private:
void Game::LoadGameResources()
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
                // Building character objects
                ResourceLoader::BuildObjects( items, m_Characters );
                break;

            case IDR_ITEMS1:
                // Building item objects
                ResourceLoader::BuildObjects( items, m_Items );
                break;
        }
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
    const std::list<int32_t>& characters = m_Map->GetLocation().GetCharacters();
    if( characters.size() )
    {
        std::list<int32_t>::const_iterator cur = characters.begin();
        std::list<int32_t>::const_iterator stop = characters.end();
        --stop;

        // Character printing starts on a new line, so no space before.
        os << "The ";
        while( cur != stop )
        {
            os << m_Characters.at( *cur )->GetName();
            if( 2 != characters.size() )
            {
                os << ", ";
            }
            ++cur;
        }

        if( 2 == characters.size() )
        {
            os << " and ";
        }
        os << m_Characters.at( *cur )->GetName();

        if( characters.size() == 1 )
        {
            os << " is";
        }
        else
        {
            os << " are";
        }
        os << " here.";
    }

    numCharacters = characters.size();

    return os;
}

std::ostream& Game::PrintItems( std::ostream &os, size_t &numItems ) const
{
    const std::list<int32_t> &items = m_Map->GetLocation().GetItems();
    if( items.size() )
    {
        std::list<int32_t>::const_iterator cur = items.begin();
        std::list<int32_t>::const_iterator stop = items.end();
        --stop;

        // Item printing starts on a new line, so no space before.
        if( items.size() > 1 )
        {
            os << "There are ";
        }
        else
        {
            os << "There is ";
        }

        while( cur != stop )
        {
            os << m_Items.at( *cur )->GetDescription();
            if( 2 != items.size() )
            {
                os << ", ";
            }
            ++cur;
        }

        if( 2 == items.size() )
        {
            os << " and ";
        }
        os << m_Items.at( *cur )->GetDescription();

        os << " here.";
    }

    numItems = items.size();

    return os;
}

bool Game::GetItemData( const std::string& name, int32_t& itemId, int32_t& itemLocId, bool& isInInventory ) const
{
    // Get the item ID for the item "name"
    itemId = InGameObject::INVALID_OBJECT;
    auto itemIt = m_ItemNameToIdMap.find( name );
    if( itemIt != m_ItemNameToIdMap.end() )
    {
        itemId = itemIt->second;
    }

    // Get the item's current location
    if( InGameObject::INVALID_OBJECT != itemId )
    {
        // Get the item's current location
        itemLocId = m_Items.at( itemId )->GetLocation();

        // Is the item in the inventory?
        auto invIt = std::find( m_Inventory.begin(), m_Inventory.end(), itemId );
        isInInventory = ( invIt != m_Inventory.end() );
    }
    else
    {
        itemLocId = InGameObject::INVALID_OBJECT;
        isInInventory = false;
    }

    // Return boolean saying if there is even an item "name"
    return ( itemId != InGameObject::INVALID_OBJECT );
}

void Game::DoItemExamination( const std::string &name, int32_t itemId, bool isInInventory ) const
{
    // Some items can't be examined unless they are in the inventory
    bool doItemExamination = false;
    if( m_Items.at( itemId )->AllowsEnvironmentExamination() )
    {
        doItemExamination = true;
    }
    else
    {
        doItemExamination = isInInventory;
    }

    if( doItemExamination )
    {
        std::cout << m_Items.at( itemId )->GetExaminationResponse() << std::endl;
    }
    else
    {
        std::cout << "You don't have the " << name << ", so you can't look at it." << std::endl;
    }
}

bool Game::GetCharacterData( const std::string& name, int32_t& characterId, int32_t& charLocId, bool& isInPosse ) const
{
    // Get the character ID for the character "name"
    characterId = InGameObject::INVALID_OBJECT;
    auto charIt = m_CharacterNameToIdMap.find( name );
    if( charIt != m_CharacterNameToIdMap.end() )
    {
        characterId = charIt->second;
    }

    if( InGameObject::INVALID_OBJECT != characterId )
    {
        // Get the character's current location
        charLocId = m_Characters.at( characterId )->GetLocation();

        // Is the item in the inventory?
        auto posseIt = std::find( m_Posse.begin(), m_Posse.end(), characterId );
        isInPosse = ( posseIt != m_Posse.end() );
    }
    else
    {
        charLocId = InGameObject::INVALID_OBJECT;
        isInPosse = false;
    }

    // Return boolean saying if there is even a character "name"
    return ( characterId != InGameObject::INVALID_OBJECT );
}

void Game::DoCharacterExamination( const std::string &name, int32_t characterId, bool isInPosse ) const
{
    // Characters can't been examined unless they are present
    bool doCharacterExamination = isInPosse;
    if( !isInPosse )
    {
        // Character isn't with us, so see if it's in the same location
        const std::list<int32_t>& characters = m_Map->GetLocation().GetCharacters();
        auto charIt = std::find( characters.begin(), characters.end(), characterId );
        doCharacterExamination = ( charIt != characters.end() );
    }

    if( doCharacterExamination )
    {
        std::cout << m_Characters.at( characterId )->GetExaminationResponse() << std::endl;
    }
     else
    {
        std::cout << "The " << name << " isn't here." << std::endl;
    }
}
