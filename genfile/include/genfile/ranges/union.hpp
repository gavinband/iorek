//          Copyright Gavin Band 2023
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_RANGES_UNION_HPP
#define GENFILE_RANGES_UNION_HPP 1

#include <vector>
#include <algorithm>

// #define DEBUG_UNION 1

namespace genfile {
	namespace ranges {
		template< typename Ranges >
		void add_union(
			Ranges* result,
			Ranges const& right
		) {
			typedef typename Ranges::value_type Range ;
			typedef typename Ranges::iterator Iterator ;
			Ranges& r = *result ;
			r.reserve( r.size() + right.size() ) ;
			r.insert( r.end(), right.begin(), right.end() ) ;
			std::sort( r.begin(),r.end() ) ;
			Iterator i = r.begin() ;
			while( i != r.end() ) {
				Iterator current_rightmost = i ;
				Iterator j = i ;
				// keep walking until the next range no longer intersects this one
				for(
					++j;
					(j != r.end()) && (j->start() <= current_rightmost->end());
					current_rightmost = j++
				) {
					// nothing to do.
				}
				Range new_range( i->start(), current_rightmost->end() ) ;
				*i = new_range ;
				Iterator next = i ;
				r.erase( ++next, j ) ;
				++i ;
			}
		}

		template< typename Ranges >
		void add_union(
			Ranges* result,
			typename Ranges::value_type const& right
		) {
			add_union(
				result,
				Ranges( 1, right )
			) ;
		}

		template< typename Ranges >
		Ranges compute_union(
			Ranges const& left,
			Ranges const& right
		) {
			typedef typename Ranges::value_type Range ;
			typedef typename Ranges::iterator Iterator ;

			Ranges result ;
			result.reserve( left.size() + right.size() ) ;
			result.insert( result.end(), left.begin(), left.end() ) ;
			add_union( &result, right ) ;
			return result ;
		}

		template< typename Ranges >
		Ranges compute_union(
			Ranges const& left,
			typename Ranges::value_type const& right
		) {
			Ranges right_as_range( 1, right ) ;
			return union_ranges( left, right_as_range ) ;
		}

		template< typename Ranges >
		Ranges compute_union(
			typename Ranges::value_type const& left,
			Ranges const& right
		) {
			Ranges left_as_range( 1, right ) ;
			return union_ranges( left_as_range, right ) ;
		}
	}
}

#endif
