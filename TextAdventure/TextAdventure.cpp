// TextAdventure.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include "Objects.h"

void GameLoop( Game& game )
{
    // Need a string comparitor for doing case insensitive compares
    typename StringCompareT comparitor;

    // Initialize the parser from the game resources
    std::shared_ptr<ParserT> parser = std::make_shared<ParserT>();

    bool quit = false;
    while( !quit )
    {
        // Write the cursor and get the player input
        std::cout << ">> ";
        std::string userInput;
        std::getline( std::cin, userInput );

        // Parse the player input
        std::stringstream ui( userInput );
        if( ParserStatusT::STATUS_PARSE_OK != parser->ParsePhrase( ui ) )
        {
            // TODO: insert smart mouthed responses here...
            std::cout << "I didn't understand that, please try again." << std::endl;
            continue;
        }

        // Figure out what the user wants to act on and what should happen.
        Game::GameObjectData objectData;
        Response *response = nullptr;
        game.GetObjectData( parser->GetLastObject(), objectData );
        if( InGameObject::INVALID != objectData.id )
        {
            // Map verb type to response type
            ResponseType type = ResponseType::RESPONSE_TYPE_INVALID;
            switch( parser->GetLastVerbType() )
            {
            case Parser::ParsedType::PARSED_TYPE_TAKE:
                type = ResponseType::RESPONSE_TYPE_TAKE;
                break;
            case Parser::ParsedType::PARSED_TYPE_EXAMINE:
                type = ResponseType::RESPONSE_TYPE_EXAMINE;
                break;
            case Parser::ParsedType::PARSED_TYPE_DISCARD:
                type = ResponseType::RESPONSE_TYPE_DISCARD;
                break;
            case Parser::ParsedType::PARSED_TYPE_THROW:
                type = ResponseType::RESPONSE_TYPE_THROW;
                break;
            case Parser::ParsedType::PARSED_TYPE_INTERACTION:
                type = ResponseType::RESPONSE_TYPE_INTERACTION;
                break;
            case Parser::ParsedType::PARSED_TYPE_ATTACK:
                type = ResponseType::RESPONSE_TYPE_ATTACK;
                break;
            case Parser::ParsedType::PARSED_TYPE_TRANSACT:
                type = ResponseType::RESPONSE_TYPE_TRANSACT;
                break;
            default:
                break;
            }

            if( ResponseType::RESPONSE_TYPE_INVALID != type )
            {
                response = game.GetBestResponse( objectData, parser->GetLastVerb(), type );
            }
        }

        // Execute the user event
        switch( parser->GetLastVerbType() )
        {
        case Parser::ParsedType::PARSED_TYPE_MOVE:
            game.OnMove( *parser );
            break;
        case Parser::ParsedType::PARSED_TYPE_TAKE:
            game.OnTake( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_EXAMINE:
            game.OnExamine( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_DISCARD:
            game.OnDiscard( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_THROW:
            game.OnThrow( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_INTERACTION:
            game.OnInteraction( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_ATTACK:
            game.OnAttack( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_TRANSACT:
            game.OnTransact( objectData, response );
            break;
        case Parser::ParsedType::PARSED_TYPE_GAME:
            // Game directive is to do one of:
            // quit, save, or load (currently only quit is supported)
            if( 0 == comparitor.compare( parser->GetLastVerb(), "quit" ) )
            {
                quit = true;
            }
            else if( 0 == comparitor.compare( parser->GetLastVerb(), "inventory" ) )
            {
                game.OnInventory();
            }
            break;
        }

        // Handle any triggers set by the response
        game.OnTrigger( response );
    };
}

int main()
{
    std::shared_ptr<Game> game = std::make_shared<Game>();
    game->OnLoad();

    // Map is all loaded up.  Start the game.
    GameLoop( *game );

    // Exiting the game loop means quitting the game.
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
