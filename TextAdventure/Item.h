#pragma once

#include "GameObject.h"

class Map;

class Item : public GameObject
{
public:
    Item( const json &item, std::weak_ptr<Map> map );

    std::ostream &PrintDescription( std::ostream &os );

private:
    bool m_IsMovable;
    int32_t m_CurLocation;
    std::string m_LongDescription;
    std::string m_Examination;
};
