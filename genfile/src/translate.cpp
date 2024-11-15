//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <map>
#include <cctype>
#include "genfile/translate.hpp"
#include "genfile/string_utils/string_utils.hpp"

//#define DEBUG 1

#if DEBUG
#include <iostream>
#endif

namespace {
	typedef std::map< std::string, char > AATable ;
	AATable const aa_table(
		{
			{"aaa", 'K' }, { "aac", 'N' }, { "aag", 'K' }, { "aat", 'N' },
			{"aca", 'T' }, { "acc", 'T' }, { "acg", 'T' }, { "act", 'T' },
			{"aga", 'R' }, { "agc", 'S' }, { "agg", 'R' }, { "agt", 'S' },
			{"ata", 'I' }, { "atc", 'I' }, { "atg", 'M' }, { "att", 'I' },
			{"caa", 'Q' }, { "cac", 'H' }, { "cag", 'Q' }, { "cat", 'H' },
			{"cca", 'P' }, { "ccc", 'P' }, { "ccg", 'P' }, { "cct", 'P' },
			{"cga", 'R' }, { "cgc", 'R' }, { "cgg", 'R' }, { "cgt", 'R' },
			{"cta", 'L' }, { "ctc", 'L' }, { "ctg", 'L' }, { "ctt", 'L' },
			{"gaa", 'E' }, { "gac", 'D' }, { "gag", 'E' }, { "gat", 'D' },
			{"gca", 'A' }, { "gcc", 'A' }, { "gcg", 'A' }, { "gct", 'A' },
			{"gga", 'G' }, { "ggc", 'G' }, { "ggg", 'G' }, { "ggt", 'G' },
			{"gta", 'V' }, { "gtc", 'V' }, { "gtg", 'V' }, { "gtt", 'V' },
			{"taa", 'X' }, { "tac", 'Y' }, { "tag", 'X' }, { "tat", 'Y' },
			{"tca", 'S' }, { "tcc", 'S' }, { "tcg", 'S' }, { "tct", 'S' },
			{"tga", 'X' }, { "tgc", 'C' }, { "tgg", 'W' }, { "tgt", 'C' },
			{"tta", 'L' }, { "ttc", 'F' }, { "ttg", 'L' }, { "ttt", 'F' }
		}
	) ;
}

namespace genfile {
	std::string translate( std::string const& sequence ) {
		std::size_t const length = sequence.size() ;
		if( length % 3 != 0 ) {
			return "?" ;
		}
		std::string result( length/3, '-' ) ;
		std::size_t i = 0 ;
		for( ; i < length; i += 3 ) {
			std::string const& codon = genfile::string_utils::to_lower( sequence.substr( i, 3 )) ;
			AATable::const_iterator where = aa_table.find( codon ) ;
#if DEBUG
			std::cerr << "TS: \"" << sequence.substr( i, 3 ) << "\": \"" << where->second << "\".\n" ;
#endif
			if( where == aa_table.end() ) {
				result[i/3] = '-' ;
			} else {
				result[i/3] = where->second ;
			}
		}
		return result ;
	}
}
