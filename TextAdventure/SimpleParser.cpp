#include "stdafx.h"

#include "ResourceLoader.hpp"
#include "SimpleParser.h"

namespace Parser
{
	SimpleParser::SimpleParser( void )
	{
		LoadJSONResources();
	}

	ParserStatus SimpleParser::ParsePhrase( std::stringstream &phrase )
	{
		// Go through the phrase looking for the verb, direct object, and an indirect object.
		// The verb and object are required, the indirect object is optional.
		ParserStatus status = ParserStatus::STATUS_PARSE_OK;
		ParseState state = ParseState::STATE_BEGIN;

		ClearLastEntries();

		bool haveDirectObject = false;
		std::string token;
		while(    status == ParserStatus::STATUS_PARSE_OK
			   && state != ParseState::STATE_END
			   && phrase >> token )
		{
			bool isArticle = IsArticle( token );
			bool isPreposition = IsPreposition( token );

			switch( state )
			{
			case ParseState::STATE_BEGIN:
				{
					// In the beginning state, the only acceptible part of speech is a verb.
					// All commands must begin with a verb; e.g., "_go_ south", "_get_ the thing", etc.
					auto found = m_VerbList.find( token );
					if( found != m_VerbList.end() )
					{
						// It's a known verb, so far so good
						m_LastVerb = found->first;
						m_LastType = found->second;

						state = ParseState::STATE_HAVE_VERB;
					}
					else
					{
						// First word must be a verb, this is a parse error
						status = ParserStatus::STATUS_EXPECTING_VERB;
					}

					// Game-type commands are single words only; e.g., quit, save, load
					if( ParsedType::PARSED_TYPE_GAME == m_LastType )
					{
						state = ParseState::STATE_END;
					}
				}
				break;
			case ParseState::STATE_HAVE_VERB:
				// Once a verb is present, any of article, preposition or direct object is acceptible
				if( isArticle )
				{
					// Parser doesn't actually do anything with articles
					state = ParseState::STATE_HAVE_ARTICLE;
				}
				else if( isPreposition )
				{
					// Preposition on the direct object is only acceptible for interactive types
					if( IsInteractiveType() )
					{
						m_LastPreposition = token;
						state = ParseState::STATE_HAVE_PREPOSITION;
					}
					else
					{
						status = ParserStatus::STATUS_EXPECTING_DIRECT_OBJECT;
					}
				}
				else
				{
					// Assume this is a direct object, the parser doesn't know if it is or isn't.
					// The game will decide if this word matches any of the objects in the game
					m_LastObject = token;
					haveDirectObject = true;
					state = ParseState::STATE_HAVE_DIRECT_OBJECT;
				}
				break;
			case ParseState::STATE_HAVE_ARTICLE:
				// After an article, it's either a direct object or indirect object
				if( isArticle || isPreposition )
				{
					// Article or preposition following the previous article, this is a parse error
					status = !haveDirectObject ? ParserStatus::STATUS_EXPECTING_DIRECT_OBJECT : ParserStatus::STATUS_EXPECING_INDIRECT_OBJECT;
				}
				else if( !haveDirectObject )
				{
					// This is the direct object
					m_LastObject = token;
					haveDirectObject = true;
					state = ParseState::STATE_HAVE_DIRECT_OBJECT;
				}
				else
				{
					// Have an indirect object.  That's all we can handle, so any other
					// text is dropped on the floor
					m_LastIndirectObject = token;
					state = ParseState::STATE_END;
				}
				break;
			case ParseState::STATE_HAVE_DIRECT_OBJECT:
				// After the direct object the parser looks for a relational connection via a preposition.
				// Words like "on", "to", "near", "above", etc.
				if( isArticle )
				{
					// Article after direct object, that's a parse error
					status = ParserStatus::STATUS_EXPECTING_PREPOSITION;
				}
				else if( isPreposition )
				{
					m_LastPreposition = token;
					state = ParseState::STATE_HAVE_PREPOSITION;
				}
				else
				{
					// Bad parse
					status = ParserStatus::STATUS_EXPECTING_PREPOSITION;
				}
				break;
			case ParseState::STATE_HAVE_PREPOSITION:
				// From the preposition, the phrase can go directly to the indirect object, or into another article
				if( isArticle )
				{
					state = ParseState::STATE_HAVE_ARTICLE;
				}
				else if( isPreposition )
				{
					// Preposition following the previous preposition is a parser error
					status = ParserStatus::STATUS_EXPECING_INDIRECT_OBJECT;
				}
				else if( haveDirectObject )
				{
					// Have an indirect object.  That's all we can handle, so any other
					// text is dropped on the floor
					m_LastIndirectObject = token;
					state = ParseState::STATE_END;
				}
				else
				{
					// Preposition on the direct object must be interaction type
					assert( IsInteractiveType() );
					m_LastObject = token;
					haveDirectObject = true;
					state = ParseState::STATE_HAVE_DIRECT_OBJECT;
				}
			}
		}

		return status;
	}

	//private:
	void SimpleParser::LoadJSONResources()
	{
		// Need to load up the resources and parse the json contained within.
		// There are three resource types:
		json verbList;
		ResourceLoader::LoadStringResource( verbList, L"KNOWN_VERBS", IDR_VERBS1 );

		// Loaded the "Verbs", create the mapping between known verbs and their action types
		for( auto it = verbList.begin(); it != verbList.end(); ++it )
		{
			std::string type = it->at( "Type" );

			// Convert the direction strings to Map::MOVE_DIRECTION enum values
			auto &verbs = it->at( "Verbs" );
			for( auto verb = verbs.begin(); verb != verbs.end(); ++verb )
			{
				m_VerbList[ verb.value() ] = GetVerbTypeEnum( type );
			}
		}
	}

	bool SimpleParser::IsPreposition( const std::string &word ) const
	{
		static const std::vector<std::string> prepositions =
		{
			"on", "to", "at", "upon"
		};

		return IsInWordList( prepositions, word );
	}
}