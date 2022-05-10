
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_FIND_HOMOPOLYMERS_HPP

#include "genfile/Fasta.hpp"

namespace genfile {
	namespace impl {
		void report_repeat(
			std::vector< char > const& repeat,
			std::size_t const start,
			std::size_t const current_length,
			std::size_t zero_based_position,
			std::function< void( uint32_t const start, uint32_t const end, std::string const& motif ) > callback
		) ;
	}

	template< typename Iterator >
	void find_homopolymers_and_short_repeats(
		Iterator begin,
		Iterator const end,
		std::function< void( uint32_t const start, uint32_t const end, std::string const& motif ) > callback,
		std::function< void( std::size_t, std::size_t ) > progress_callback = std::function< void (std::size_t, std::size_t) >()
	) {
		std::size_t const max_length = 3 ;
		std::vector< std::vector< char > > repeats ;
		std::vector< std::size_t > counters ;
		std::vector< std::size_t > starts ;
		std::string repeat_string ;
		for( uint32_t i = 0; i < max_length; ++i ) {
			repeats.push_back( std::vector< char >( i+1, ' ' )) ;
			counters.push_back( 0 ) ;
			starts.push_back( 0 ) ;
		}
	
		std::size_t const sequence_length = std::distance( begin, end ) ;
		std::size_t x = 0 ;
		for( ; begin != end; ++begin, ++x ) {
			char base = ::toupper( *begin ) ;
			for( uint32_t i = 0; i < max_length; ++i ) {
				// if counter < repeat unit length, then this is the first view of the repeat.
				// if counter >= repeat unit length, then we are repeating.
				auto& repeat = repeats[i] ;
				std::size_t const repeat_unit_length = repeat.size() ;
				auto& counter = counters[i] ;
				auto& start = starts[i] ;
				std::size_t const current_length = counter - start ;

				// Algorithm example: take sequence
				// A T G A T G A C T A C  T ...
				// 0 1 2 3 4 5 6 7 8 9 10 11
				// with repeat unit length L = 3.
				// We accumulate sequence until the repeat length is filled.
				// At this point we test the next base against the next repeat base.
				// If they differ, the potential repeat is over.  If it is a genuine repeat (more than two copies) we output.
				// To start a new repeat, we set start to (L-1) bases back to account for possible
				// overlapping repeats, as in the example.
				if( (current_length >= repeat_unit_length) && (base != repeat[counter % repeat_unit_length]) ) {
					if( current_length >= 2 * repeat.size() ) {
						impl::report_repeat( repeat, start, current_length, x, callback ) ;
					}
					start = (counter + 1) % repeat_unit_length ;
					counter = start + repeat_unit_length-1 ;
				}
				repeat[counter % repeat_unit_length] = base ;
				++counter ;
			}
			progress_callback( x, sequence_length ) ;
		}
		// Handle the last repeat.
		for( uint32_t i = 0; i < max_length; ++i ) {
			auto& repeat = repeats[i] ;
			auto& counter = counters[i] ;
			auto& start = starts[i] ;
			std::size_t const current_length = counter - start ;
			if( current_length >= 2 * repeat.size() ) {
				impl::report_repeat( repeat, start, current_length, x, callback ) ;
			}
		}
		progress_callback( sequence_length, sequence_length ) ;
	}

	void find_homopolymers_and_short_repeats(
		Fasta const& fasta,
		std::function< void( uint32_t const start, uint32_t const end, std::string const& motif ) > callback,
		std::function< void( std::size_t, std::size_t ) > progress_callback = std::function< void (std::size_t, std::size_t) >()
	) ;
}

#endif
