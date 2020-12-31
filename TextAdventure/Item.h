#pragma once

class Item : public InGameObject
{
public:
    Item( const json &item );

    std::ostream &PrintDescription( std::ostream &os );

private:
    bool m_IsMovable;
    int32_t m_CurLocation;
    std::string m_LongDescription;
    std::string m_Examination;
};
