#pragma once

class Item : public InGameObject
{
public:
    Item( const json &item );

private:
    // Members specific to the item class
    bool m_IsMovable;
};
