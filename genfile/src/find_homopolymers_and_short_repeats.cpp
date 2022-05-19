
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include "genfile/Fasta.hpp"
#include "genfile/find_homopolymers_and_short_repeats.hpp"

namespace genfile {
	namespace impl {
		void report_repeat(
			std::vector< char > const& repeat,
			std::size_t const start,
			std::size_t const current_length,
			std::size_t zero_based_position,
			std::function< void( uint32_t const start, uint32_t const end, std::string const& motif ) > callback
		) {
			std::size_t const repeat_unit_length = repeat.size() ;
			std::string repeat_string( repeat_unit_length, ' ' ) ;
			std::copy( repeat.begin() + start, repeat.end(), repeat_string.begin() ) ;
			std::copy( repeat.begin(), repeat.begin() + start, repeat_string.begin() + repeat_unit_length - start ) ;
			// Avoid sequences of Ns or other missing base data
			bool only_proper_nucleotides = ( repeat_string.find_first_not_of( "ACGT" ) == std::string::npos ) ;
			// Avoid listing homopolymers as di- or tri-nucleotide repeats
			// TODO: fix the following line to work for arbitrary repeat length.
			bool valid = (
				(repeat.size() == 1 )
				|| ( repeat.size() == 2 && repeat[1] != repeat[0] )
				|| ( repeat.size() == 3 && ( repeat[1] != repeat[0] || repeat[2] != repeat[0] ))
			) ;
			if( valid && only_proper_nucleotides ) {
				// report in 0-based, half open coords
				callback( zero_based_position - current_length, zero_based_position, repeat_string ) ;
			}
		} ;
	}

	void find_homopolymers(
		Fasta const& fasta,
		std::function< void( std::string const&, uint32_t const start, uint32_t const end, std::string const& motif ) > callback,
		std::function< void( std::size_t, std::size_t ) > progress_callback = std::function< void (std::size_t, std::size_t) >()
	) {
		std::vector< std::string > const& sequence_ids = fasta.sequence_ids() ;
		std::size_t count = 0 ;
		for( auto sequence_id: sequence_ids ) {
			genfile::Fasta::PositionedSequenceRange const& contig = fasta.get_sequence( sequence_id ) ;
			find_homopolymers_and_short_repeats(
				contig.sequence().begin(),
				contig.sequence().end(),
				[&]( uint32_t start, uint32_t end, std::string const& repeat ) {
					callback( sequence_id, start, end, repeat ) ;
				}
			) ;
			progress_callback( ++count, sequence_ids.size() ) ;
		}
	}
}
