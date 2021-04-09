#pragma once

namespace Parser
{
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

	class ParserBase
	{
		using ParserMap = std::map<std::string, ParsedType, StringCompareT>;
		struct ParsedTypeToNameMap
		{
			ParsedType	type;
			std::string name;
		};

	public:
		ParserBase( void );
		
		virtual ParserStatus ParsePhrase( std::stringstream &phrase ) = 0;

		ParsedType GetLastVerbType( void ) const;
		const std::string &GetLastVerb( void ) const;
		const std::string &GetLastObject( void ) const;
		const std::string &GetLastPreposition( void ) const;
		const std::string &GetLastIndirectObject( void ) const;

		const std::string& GetVerbTypeName( ParsedType type ) const;
		ParsedType GetVerbTypeEnum( std::string& type ) const;

	protected:
		// This needs to stay in sync with the ParsedType enum class and the object JSON inputs
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

		void ClearLastEntries( void );
		bool IsArticle( const std::string& word ) const;
		bool IsInWordList( const std::vector<std::string> &list, const std::string &word ) const;
		bool IsInteractiveType( void );

		virtual bool IsPreposition( const std::string &word ) const = 0;

		ParserMap   m_VerbList;
		ParsedType  m_LastType;

		std::string m_LastVerb;
		std::string m_LastObject;
		std::string m_LastPreposition;
		std::string m_LastIndirectObject;

	};
}
