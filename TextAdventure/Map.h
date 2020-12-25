#pragma once

class Character;
class Item;
class Location;

class Map : public std::enable_shared_from_this<Map>
{
public:
    enum class MoveDirection
    {
        MOVE_INVALID = -1,
        MOVE_NORTH = 0,
        MOVE_NORTH_EAST,
        MOVE_EAST,
        MOVE_SOUTH_EAST,
        MOVE_SOUTH,
        MOVE_SOUTH_WEST,
        MOVE_WEST,
        MOVE_NORTH_WEST,
        MOVE_UP,
        MOVE_DOWN,
        MAX_DIRECTIONS
    };

    void LoadMap( std::weak_ptr<Map> weakThis );

    std::shared_ptr<Character> &GetCharacter( int32_t characterId );
    std::shared_ptr<Item> &GetItem( int32_t itemID );

    int32_t GetLocationId( void );
    std::shared_ptr<Location> &GetLocation( int32_t locationId = -1 );

private:
    void LoadJSONResources( std::weak_ptr<Map> weakThis );
    template<typename Obj>
    void BuildObjects( const json &objJson, std::weak_ptr<Map> weakThis );

    void PlaceCharacters( void );
    void PlaceItems( void );
    void SetStartingLocation( void );

    int32_t                                       m_CurLocation;

    std::map<int32_t, std::shared_ptr<Character>> m_Characters;
    std::map<int32_t, std::shared_ptr<Item>>      m_Items;
    std::map<int32_t, std::shared_ptr<Location>>  m_Locations;
};
