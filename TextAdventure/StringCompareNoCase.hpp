#pragma once

namespace StringComparitors
{
	class StringCompareNoCase
	{
	public:
		inline bool operator()( const std::string &lhs, const std::string &rhs ) const
		{
			int comp = compare( lhs, rhs );
			return ( comp < 0 );
		}

		inline int compare( const std::string &a, const std::string &b ) const
		{
			// Yeah, pretty much only works with ASCII.
			std::string a0( a ), b0( b );
			std::transform( a.begin(), a.end(), a0.begin(), ::tolower );
			std::transform( b.begin(), b.end(), b0.begin(), ::tolower );

			return a0.compare( b0 );
		}
	};
}
