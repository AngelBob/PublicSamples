// TextAdventure.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"

#include "Location.h"
#include "Map.h"

void GameLoop( std::shared_ptr<Map> map )
{
    bool quit = false;
    do
    {
        // First, display a description of the location
        std::stringstream ss;

        std::shared_ptr<Location>& curLocation = map->GetLocation();
        curLocation->PrintName( ss );
        ss << "\n";
        curLocation->PrintDescription( ss );
        curLocation->PrintCharacters( ss );
        curLocation->PrintItems( ss );
        ss << "\n";

        std::string message( ss.str() );
        std::cout << message;

        // Next, get the player's action

        quit = true;
    } while( !quit );
}

int main()
{
    std::shared_ptr<Map> map = std::make_shared<Map>();
    std::weak_ptr<Map> weakMap = map;
    map->LoadMap( weakMap );

    // Map is all loaded up.  Start the game.
    GameLoop( map );

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
