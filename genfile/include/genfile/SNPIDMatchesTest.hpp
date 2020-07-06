
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_SNPIDMATCHES_TEST_HPP
#define GENFILE_SNPIDMATCHES_TEST_HPP

#include <string>
#include "VariantIdentifyingDataTest.hpp"

namespace genfile {
	struct SNPIDMatchesTest: public VariantIdentifyingDataTest
	{
	public:
		SNPIDMatchesTest( std::string const& expression ) ;
		bool operator()( VariantIdentifyingData const& data ) const ;
		std::string display() const ;
	private:
		void setup( std::string const& ) ;
		bool match( std::string const& ) const ;
	private:
		enum Type { eSNPID = 1, eRSID = 2, eEITHER = 3 } ;	
		Type m_type ;
		std::string m_expression, m_prefix, m_suffix ;
		char m_wildcard_char ;
		bool m_have_wildcard ;
	} ;
}

#endif

