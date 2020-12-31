#pragma once

class Character : public InGameObject
{
public:
	Character( const json &character );

	std::string &GetResponse( void );

private:
	int32_t					 m_CurLocationId;
	int32_t					 m_CurResponse;
	std::vector<std::string> m_Responses;
};
