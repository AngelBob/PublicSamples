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
    }

    // Cycle through all of the items next.
    for( auto &item : m_Items )
    {
        // The first "item" is the invalid item, and it gets placed into the invalid location.
        Location &location = m_Map->GetLocation( item->GetLocation() );
        location.AddItem( item->GetObjectId() );
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
        // Move was good, describe the scene
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

void Game::OnInteraction( const ParserT& parser )
{
    // Get the character(s) from the current location.
    // If there is more than one, need to tell the user to be more specific.
    const std::list<int32_t>& characterIds = m_Map->GetLocation().GetCharacters();
    if( characterIds.size() == 0 )
    {
        std::cout << "Who are you talking to?  There's no one here" << std::endl;
    }
    else if( characterIds.size() > 1 )
    {
        std::cout << "Who are you talking to?  There are several people present" << std::endl;
    }
    else
    {
        // The intereseting bit.. have the character respond (assuming the user is
        // wanting to talk to someone that's actually present).
        const std::unique_ptr<Character>& character = m_Characters[ characterIds.front() ];
        const std::string& name = parser.GetLastObject();

        if( name.empty() || name == character->GetName() )
        {
            character->OnInteraction( std::cout ) << std::endl;
        }
        else
        {
            std::cout << character->GetName() << " isn't here." << std::endl;
        }
    }
}

void Game::DescribeScene( void )
{
    std::stringstream ss;

    const Location& curLocation = m_Map->GetLocation();
    ss << curLocation.GetName();
    ss << "\n";
    curLocation.PrintDescription( ss, curLocation.LongOrShortDescription() );

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
        //{ L"ITEMS", IDR_ITEMS1 },
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
            os << " you see ";

            m_Map->GetLocation( cur->second ).PrintDescription( os, LocationDesc::DESCRIPTION_ASSEEN );

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
            os << "There is a ";
        }

        while( cur != stop )
        {
            m_Items.at( *cur )->PrintDescription( os );
            if( 2 != items.size() )
            {
                os << ", ";
            }
            ++cur;
        }

        if( 2 == items.size() )
        {
            os << " and a ";
        }
        m_Items.at( *cur )->PrintDescription( os );

        os << " here.";
    }

    numItems = m_Items.size();

    return os;
}
