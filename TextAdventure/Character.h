#pragma once

#include "GameObject.h"

class Map;

class Character : public GameObject
{
public:
	Character( const json &character, std::weak_ptr<Map> map );

	std::string &GetResponse( void );

private:
	int32_t					 m_CurLocationId;
	int32_t					 m_CurResponse;
	std::vector<std::string> m_Responses;
};
