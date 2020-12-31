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
        Location &location = m_Map->GetLocation( character->GetLocation() );
        location.AddCharacter( character->GetObjectId() );
    }

    // Cycle through all of the items next.
    for( auto &item : m_Items )
    {
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

void Game::DescribeScene( void )
{
    std::stringstream ss;

    const Location& curLocation = m_Map->GetLocation();
    curLocation.PrintName( ss );
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
#if 0 // not quite ready to load up the items and characters
    // Need to load up the character and item resources
    static const ResourceList resources[ 2 ] = {
        { L"CHARACTERS", IDR_CHARACTERS1 },
        { L"ITEMS", IDR_ITEMS1 },
    };

    for( int32_t idx = 0; idx < _countof( resources); ++idx )
    {
        json items;
        ResourceLoader::LoadStringResource( items, resources[ idx ].name, resources[ idx ].id );

        switch( idx )
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
#endif
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

        os << "  The ";
        while( cur != stop )
        {
            m_Characters.at( *cur )->PrintName( os );
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
        m_Characters.at( *cur )->PrintName( os );

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

        os << "  ";
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
