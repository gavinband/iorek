//          Copyright Gavin Band 2023
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_RANGES_INVERT_HPP
#define GENFILE_RANGES_INVERT_HPP 1

#include <vector>
#include <algorithm>

namespace genfile {
	namespace ranges {
		template< typename Ranges >
		Ranges invert_within(
			Ranges const& ranges,
			typename Ranges::value_type const& within
		) {
			typedef typename Ranges::value_type Range ;
			typedef typename Ranges::iterator Iterator ;
			Ranges result ;
			if( ranges.size() == 0 ) {
				return Ranges( 1, within ) ;
			}
			auto a = within.start() ;
			for( auto range: ranges ) {
				if( range.start() > a ) {
					result.push_back( Range( a, range.start() )) ;
					a = range.end() ;
				}
			}
			if( a < within.end() ) {
				result.push_back( Range( a, within.end() )) ;
			}
			return result ;
		}
	}
}

#endif
