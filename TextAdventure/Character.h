#pragma once

class Character : public InGameObject
{
public:
	Character( const json &character );

	// Event handlers
	std::ostream& OnInteraction( std::ostream& os );

private:
	int32_t					 m_CurLocationId;
	int32_t					 m_CurResponse;

	std::vector<std::string> m_InteractionResponses;
};
