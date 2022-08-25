
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_REPEATS_HOMOPOLYMERTRACTWALKER
#define GENFILE_REPEATS_HOMOPOLYMERTRACTWALKER 1

#include "repeat_tracts.hpp"

namespace genfile {
	namespace repeats {
		// HomopolymerTractWalker keeps track of homopolymer tracts
		// as we walk along a sequence
		// Thinking of a position as between two bases, at each position it keeps track of:
		// the homopolymer (of length 1 or more) containing the base immediately to the
		// left and the homopolymer immediately to the right.
		// (These may be the same).
		template< typename SequenceIterator >
		struct HomopolymerTractWalker {
			struct Tract {
				Tract( Tract const& other ):
					m_begin( other.m_begin ),
					m_end( other.m_end )
				{}
				Tract( SequenceIterator const begin, SequenceIterator const end ):
					m_begin( begin ),
					m_end( end )
				{}
				Tract& operator=( Tract const& other ) {
					m_begin = other.m_begin ;
					m_end = other.m_end ;
					return *this ;
				}
			public:
				std::size_t size() const { return std::distance( m_begin, m_end ) ; }
				SequenceIterator const& begin() const { return m_begin ; } ;
				SequenceIterator const& end() const { return m_end ; } ;

			private:
				SequenceIterator m_begin ;
				SequenceIterator m_end ;
			} ;

			HomopolymerTractWalker(
				SequenceIterator const contig_begin,
				SequenceIterator const contig_end,
				SequenceIterator position
			):
				m_contig_begin( contig_begin ),
				m_contig_end( contig_end ),
				m_position( position )
			{
				initialise() ;
			}

			HomopolymerTractWalker( HomopolymerTractWalker const& other ):
				m_contig_begin( other.m_contig_begin ),
				m_contig_end( other.m_contig_end ),
				m_position( other.m_position ),
				m_l_begin( other.m_l_begin ),
				m_l_end( other.m_l_end ),
				m_r_begin( other.m_r_begin ),
				m_r_end( other.m_r_end )
			{}

			HomopolymerTractWalker& operator=( HomopolymerTractWalker const& other ) {
				m_contig_begin = other.m_contig_begin ;
				m_contig_end = other.m_contig_end ;
				m_position = other.m_position ;
				m_l_begin =  other.m_l_begin ;
				m_l_end =  other.m_l_end ;
				m_r_begin =  other.m_r_begin ;
				m_r_end =  other.m_r_end ;
				return *this ;
			}
				
			SequenceIterator const contig_begin() const { return m_contig_begin ; }
			SequenceIterator const contig_end() const { return m_contig_end ; }
			SequenceIterator position() const { return m_position ; }

			Tract left_tract() const {
				return Tract( m_l_begin, m_l_end ) ;
			}

			Tract right_tract() const {
				return Tract( m_r_begin, m_r_end ) ;
			}

			// Invariant:
			// m_l_begin ... m_l_end points to the homopolymer range of length at least 1
			// containing the base immediately before this position.
			// If the position is the start of the sequence, the range is empty.
			// m_r_begin ... m_r_end points to the homopolymer range of length at least 1
			// containing the base at this position.
			// If m_r_begin is the end of the sequence, this range is empty.
			void operator++() {
				// precondition: m_position lies in [m_contig_start, m_contig_end).

				++m_position ;
				// Due to the invariant we can always transfer right tract
				// to left tract.  (If we are internal to a tract they are already the same.)
				m_l_begin = m_r_begin ;
				m_l_end = m_r_end ;

				// If we reached the end of a tract, we need to compute
				// the new right tract.
				if( m_position == m_l_end ) {
					m_r_begin = m_position ;
					m_r_end = m_position ;
					if( m_position != m_contig_end ) {
						SequenceIterator x = m_position ;
						++x ;
						m_r_end = genfile::repeats::expand_rightwards(
							m_position, x, m_contig_end
						) ;
					} ;
				}
			}

			void operator+=( int length ) {
				for( int i = 0; i < length; ++i ) {
					operator++() ;
				}
			}

		private:
			SequenceIterator const m_contig_begin ;
			SequenceIterator const m_contig_end ;
			SequenceIterator m_position ;
	
			SequenceIterator m_l_begin ;
			SequenceIterator m_l_end ;
			SequenceIterator m_r_begin ;
			SequenceIterator m_r_end ;

			void initialise() {
				m_l_end = m_r_begin = m_position ;
				if( m_position == m_contig_begin ) {
					m_l_begin = m_position ;
				} else {
					SequenceIterator x = m_position ;
					--x ;
					m_l_begin = genfile::repeats::expand_leftwards(
						x, m_position, m_contig_begin
					) ;
				}
		
				if( m_position == m_contig_end ) {
					m_r_end = m_position ;
				} else {
					SequenceIterator x = m_position ;
					++x ;
					m_r_end = genfile::repeats::expand_rightwards(
						m_position, x, m_contig_end
					) ;
				}
			}
		} ;
	}
}

#endif
