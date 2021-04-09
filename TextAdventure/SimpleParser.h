#pragma once

#include "Parser.h"

namespace Parser
{
	class SimpleParser : public ParserBase
	{
		using ParserMap = std::map<std::string, ParsedType, StringCompareT>;
		struct ParsedTypeToNameMap
		{
			ParsedType	type;
			std::string name;
		};

	public:
		SimpleParser( void );
		ParserStatus ParsePhrase( std::stringstream &phrase );

	private:
		void LoadJSONResources( void );
		bool IsPreposition( const std::string &word ) const;
	};
}
