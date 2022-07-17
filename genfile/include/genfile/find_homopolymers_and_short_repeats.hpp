
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

	// Find all homopolymer, di- and tri-nucleotide repeats with at least 2 repeat units
	// in the sequence supplied by the pair of iterators.
	// Results are reported via the callback in 0-based, half-open coordinates, i.e. the first base is at 0 and the range start = 10, end = 15 representes a span of 5 bases.
	// If repeat units overlap (e.g. CATCATAGTAG) then both overlapping ranges are reported.
	// Non-nucleotide bases (ACGT) are ignored.
	// Homopolymers are not reported as di- or tri-nucleotide repeats (e.g. AAA is not a repeat unit.)
	template< typename Iterator >
	void find_homopolymers_and_short_repeats(
		Iterator begin,
		Iterator const end,
		std::size_t const minimum_length,
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
				// A T G A T G A C G A C  G ...
				// 0 1 2 3 4 5 6 7 8 9 10 11
				// with repeat unit length L = 3.
				// We accumulate sequence until the repeat length is filled.
				// At this point we test the next base against the next repeat base.
				// If they differ, the potential repeat is over.  If it is a genuine repeat (more than two copies) we output.
				// To start a new repeat, we set start to (L-1) bases back to account for possible
				// overlapping repeats.  For the above example, we should output:
				// (ATG) repeat for 7 bases
				// (GAC) repeat for 7 bases
				// etc. - noting that these overlap.
				if( (current_length >= repeat_unit_length) && (base != repeat[counter % repeat_unit_length]) ) {
					if(
						(current_length >= 2 * repeat.size())
						&& (current_length >= minimum_length)
					) {
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

	// Find all homopolymer, di- and tri-nucleotide repeats with at least 2 repeat units
	// in the given fasta file.
	// Results are reported via the callback in 0-based, half-open coordinates, i.e. the first base is at 0 and the range start = 10, end = 15 representes a span of 5 bases.
	// If repeat units overlap (e.g. CATCATAGTAG) then both overlapping ranges are reported.
	// Non-nucleotide bases (ACGT) are ignored.
	// Homopolymers are not reported as di- or tri-nucleotide repeats (e.g. AAA is not a repeat unit.)
	void find_homopolymers_and_short_repeats(
		Fasta const& fasta,
		std::size_t const minimum_length,
		std::function< void( uint32_t const start, uint32_t const end, std::string const& motif ) > callback,
		std::function< void( std::size_t, std::size_t ) > progress_callback = std::function< void (std::size_t, std::size_t) >()
	) ;
}

#endif
