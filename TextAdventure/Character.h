#pragma once

class Character : public InGameObject
{
public:
	Character( const json &character );

	// Event handlers
	std::ostream& OnInteraction( std::ostream& os );

private:
	// Members specific to the character class
	int32_t m_CurResponse;
};
