
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_REVERSE_COMPLEMENT_HPP
#define GENFILE_REVERSE_COMPLEMENT_HPP

#include <string>
#include <algorithm>

namespace genfile {
	std::string reverse_complement( std::string const& sequence ) ;

	template< typename Iterator >
	std::string reverse_complement( Iterator begin, Iterator const end ) {
		char map[256] ;
		std::fill( &map[0], &map[0] + 256, 'N' ) ;
		map[int('A')] = 'T' ;
		map[int('T')] = 'A' ;
		map[int('C')] = 'G' ;
		map[int('G')] = 'C' ;
		map[int('a')] = 't' ;
		map[int('t')] = 'a' ;
		map[int('c')] = 'g' ;
		map[int('g')] = 'c' ;
		std::string result( begin, end ) ;
		for( std::size_t i = 0; i < result.size(); ++i ) {
			result[i] = map[ int(result[i]) ] ;
		}
		std::reverse( result.begin(), result.end() ) ;
		return result ;
	}
}

#endif
