#pragma once

class Character;
class Item;
class Location;

enum class MoveDirection
{
    MOVE_INVALID = 0,
    MIN_DIRECTIONS = 1,
    MOVE_NORTH = 1,
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

class Map : public std::enable_shared_from_this<Map>
{
    struct MoveDirectionToNameMap
    {
        MoveDirection type;
        std::string name;
    };

public:
    // Event handlers
    void OnLoad( void );
    void OnMove( const std::string &destination );

    // Accessors
    Location& GetLocation( int32_t locationId = -1 );
    Location& GetLocation( const std::string& name );

    const std::string& GetDirectionName( MoveDirection direction );
    MoveDirection GetDirectionEnum( const std::string& direction );

    int32_t FindObject( int32_t obejctId ) const;

private:
    inline static const MoveDirectionToNameMap m_MoveDirectionToNameMap[ 11 ] = {
        { MoveDirection::MOVE_INVALID, "invalid" },
        { MoveDirection::MOVE_NORTH, "north" },
        { MoveDirection::MOVE_NORTH_EAST, "north east" },
        { MoveDirection::MOVE_EAST, "east" },
        { MoveDirection::MOVE_SOUTH_EAST, "south east" },
        { MoveDirection::MOVE_SOUTH, "south" },
        { MoveDirection::MOVE_SOUTH_WEST, "south west" },
        { MoveDirection::MOVE_WEST, "west" },
        { MoveDirection::MOVE_NORTH_WEST, "north west" },
        { MoveDirection::MOVE_UP, "up" },
        { MoveDirection::MOVE_DOWN, "down" },
    };
    std::map<std::string, MoveDirection> m_NameToMoveDirectionMap;

    void LoadLocationResources( json& locations );
    void SetLocation( int32_t newLocation );

    int32_t                                 m_CurLocation;

    std::map<std::string, int32_t, StringCompareT> m_LocationNameToIdMap;
    std::vector<std::shared_ptr<InGameObject>>     m_Locations;
};
