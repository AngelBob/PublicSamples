#pragma once

namespace Parser
{
	class StringCompareNoCase
	{
	public:
		bool operator()( const std::string &lhs, const std::string &rhs ) const;
		int  compare( const std::string &a, const std::string &b ) const;
	};

	enum class ParsedType
	{
		PARSED_TYPE_INVALID = 0,
		PARSED_TYPE_MOVE = 1,
		PARSED_TYPE_MIN_TYPE = 1,
		PARSED_TYPE_TAKE,
		PARSED_TYPE_EXAMINE,
		PARSED_TYPE_DISCARD,
		PARSED_TYPE_THROW,
		PARSED_TYPE_INTERACTION,
		PARSED_TYPE_ATTACK,
		PARSED_TYPE_TRANSACT,
		PARSED_TYPE_GAME,
		PARSED_TYPE_MAX_TYPES
	};

	enum class ParserStatus
	{
		STATUS_PARSE_OK = 0,
		STATUS_BAD_PARSE,
		STATUS_EXPECTING_VERB,
		STATUS_EXPECTING_DIRECT_OBJECT,
		STATUS_EXPECTING_PREPOSITION,
		STATUS_EXPECING_INDIRECT_OBJECT,
	};

	enum class ParseState
	{
		STATE_BEGIN = 0,
		STATE_HAVE_VERB,
		STATE_HAVE_ARTICLE,
		STATE_HAVE_DIRECT_OBJECT,
		STATE_HAVE_PREPOSITION,
		STATE_HAVE_INDIRECT_OBJECT,
		STATE_END
	};

	class SimpleParser
	{
		using ParserMap = std::map<std::string, ParsedType, StringCompareNoCase>;
		struct ParsedTypeToNameMap
		{
			ParsedType	type;
			std::string name;
		};

	public:
		SimpleParser( void );
		ParserStatus ParsePhrase( std::stringstream &phrase );

		ParsedType GetLastVerbType( void ) const;
		const std::string& GetLastVerb( void ) const;
		const std::string &GetLastObject( void ) const;
		const std::string &GetLastIndirectObject( void ) const;

		const std::string& GetVerbTypeName( ParsedType type );
		ParsedType GetVerbTypeEnum( std::string& type );

	private:
		inline static const ParsedTypeToNameMap m_TypeToNameMap[ 10 ] = {
			{ ParsedType::PARSED_TYPE_INVALID, "Invalid" },
			{ ParsedType::PARSED_TYPE_MOVE, "Move" },
			{ ParsedType::PARSED_TYPE_TAKE, "Take" },
			{ ParsedType::PARSED_TYPE_EXAMINE, "Examine" },
			{ ParsedType::PARSED_TYPE_DISCARD, "Discard" },
			{ ParsedType::PARSED_TYPE_THROW, "Throw" },
			{ ParsedType::PARSED_TYPE_INTERACTION, "Interaction" },
			{ ParsedType::PARSED_TYPE_ATTACK, "Attack" },
			{ ParsedType::PARSED_TYPE_TRANSACT, "Transact" },
			{ ParsedType::PARSED_TYPE_GAME, "Game" },
		};
		std::map<std::string, ParsedType> m_NameToTypeMap;

		void LoadJSONResources( void );
		void ClearLastEntries( void );
		bool IsArticle( std::string& word );
		bool IsPreposition( std::string& word );

		ParserMap   m_VerbList;
		ParsedType  m_LastType;

		std::string m_LastVerb;
		std::string m_LastObject;
		std::string m_LastPreposition;
		std::string m_LastIndirectObject;

	};
}
