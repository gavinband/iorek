
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stddef.h>
#include <utility>
#include <limits>
#include "genfile/GenomePosition.hpp"
#include "genfile/string_utils.hpp"
#include "genfile/Error.hpp"

namespace genfile {
	GenomePosition::GenomePosition( std::string const& position_spec ) {
		std::vector< std::string > bits = string_utils::split_and_strip( position_spec, ":", " " ) ;
		if( bits.size() > 2 || bits.size() == 0 ) {
			throw BadArgumentError( "GenomePosition::GenomePosition()", "position_spec = \"" + position_spec + "\"" ) ;
		}
		else if( bits.size() == 1 ) {
			m_data.first = Chromosome() ;
			m_data.second = string_utils::to_repr< Position >( bits[0] ) ;
		}
		else {
			m_data.first = string_utils::to_repr< Chromosome >( bits[0] ) ;
			m_data.second = string_utils::to_repr< Position >( bits[1] ) ;
		}
	}
	
	std::ostream& operator<<( std::ostream& oStream, GenomePosition const& pos ) {
		if( pos.chromosome() != Chromosome() ) {
			oStream << pos.chromosome() << ":" ;
		}
		return oStream << pos.position() ;
	}

	std::istream& operator>>( std::istream& iStream, GenomePosition& pos ) {
		std::string s;
		iStream >> s ;
		GenomePosition gp = GenomePosition( s ) ;
		pos = gp ;
		return iStream ;
	}

	Position GenomePosition::get_max_position( Chromosome chromosome ) {
		// Report largest number in 32-bit signed integer, minus 1.
		// This matches htslib, which uses INT_MAX to denote a missing chromosome
		return Position(INT_MAX) - 1 ;
	}

	Position GenomePosition::get_min_position( Chromosome chromosome ) {
		return 0u;
	}
}
