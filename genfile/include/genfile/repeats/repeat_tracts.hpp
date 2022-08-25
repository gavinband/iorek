
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_REPEATS_HPP
#define GENFILE_REPEATS_HPP 1

namespace genfile {
	namespace repeats {
		template< typename Iterator >
		Iterator expand_leftwards(
			Iterator start,
			Iterator const end,
			Iterator const contig_begin
		) {
			Iterator const rus = start ;   // track original start of repeat unit
			Iterator rup = start ;         // location in original repeat unit
			Iterator p = start ;           // location in sequence
			while( p != contig_begin ) {
				--p ;
				// move to previous base in repeat unit
				rup = ( rup == rus ? end : rup ) ;
				--rup ;
				if( *p != *rup ) {
					break ;
				}
				--start ;
			}
			return start ;
		}

		template< typename Iterator >
		Iterator expand_rightwards(
			Iterator const start,
			Iterator end,
			Iterator const contig_end
		) {
			Iterator const rue = end ;     // track original end of repeat unit
			Iterator rup = start ;         // base in original repeat unit
			while( end != contig_end ) {
				if( *end != *rup ) {
					break ;
				}
				++end ;
				++rup ;
				rup = ( rup == rue ? start : rup ) ;
			}
			return end ;
		}

		// Example.  Consider this call:
		// supposed we call expand( "TGAGAGAGT", "GA", 0, 5, 7 )
		// the function will return:
		// start = 1
		// end = 8
		// k = 1
		//
		template< typename Iterator >
		void expand(
			Iterator const contig_begin,
			Iterator const contig_end,
			Iterator& start,
			Iterator& end
		) {
			Iterator const repeat_start = start ;
			Iterator tract_start = expand_leftwards( repeat_start, end, contig_begin ) ;
			Iterator tract_end = expand_rightwards( repeat_start, end, contig_end ) ;
			start = tract_start ;
			end = tract_end ;
		}
	}

}

#endif

