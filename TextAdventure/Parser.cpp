#include "stdafx.h"

#include "Parser.h"

namespace Parser
{
	ParserBase::ParserBase()
		: m_LastType( ParsedType::PARSED_TYPE_INVALID )
	{
		// Load name to type enum map
		for( int32_t idx = 0; idx < static_cast< int32_t >( ParsedType::PARSED_TYPE_MAX_TYPES ); ++idx )
		{
			m_NameToTypeMap.insert( std::make_pair( m_TypeToNameMap[ idx ].name, m_TypeToNameMap[ idx ].type ) );
		}
	}

	ParsedType ParserBase::GetLastVerbType( void ) const
	{
		return m_LastType;
	}

	const std::string& ParserBase::GetLastVerb( void ) const
	{
		return m_LastVerb;
	}

	const std::string& ParserBase::GetLastObject( void ) const
	{
		return m_LastObject;
	}

	const std::string& ParserBase::GetLastIndirectObject( void ) const
	{
		return m_LastIndirectObject;
	}

	const std::string& ParserBase::GetVerbTypeName( ParsedType type ) const
	{
		assert( type >= ParsedType::PARSED_TYPE_INVALID && type < ParsedType::PARSED_TYPE_MAX_TYPES );
		if( type < ParsedType::PARSED_TYPE_INVALID
			|| type >= ParsedType::PARSED_TYPE_MAX_TYPES )
		{
			type = ParsedType::PARSED_TYPE_INVALID;
		}

		return m_TypeToNameMap[ static_cast< int32_t >( type ) ].name;
	}

	ParsedType ParserBase::GetVerbTypeEnum( std::string &name ) const
	{
		ParsedType type = ParsedType::PARSED_TYPE_INVALID;
		auto found = m_NameToTypeMap.find( name );
		if( found != m_NameToTypeMap.end() )
		{
			type = found->second;
		}

		return type;
	}

	//private:
	void ParserBase::ClearLastEntries( void )
	{
		m_LastType = ParsedType::PARSED_TYPE_INVALID;
		m_LastVerb.clear();
		m_LastObject.clear();
		m_LastPreposition.clear();
		m_LastIndirectObject.clear();
	}

	bool ParserBase::IsArticle( const std::string &word ) const
	{
		static const std::vector<std::string> articles =
		{
			"the", "a", "an"
		};

		return IsInWordList( articles, word );
	}

	bool ParserBase::IsInWordList( const std::vector<std::string> &list, const std::string &word ) const
	{
		bool isInList = false;

		StringCompareT comparitor;
		for( auto &entry : list )
		{
			if( 0 == comparitor.compare( word, entry ) )
			{
				isInList = true;
				break;
			}
		}

		return isInList;
	}

	bool ParserBase::IsInteractiveType( void )
	{
		bool isInteractiveType =
               ( ParsedType::PARSED_TYPE_INTERACTION == m_LastType )
			|| ( ParsedType::PARSED_TYPE_EXAMINE == m_LastType );

		return isInteractiveType;
	}
}