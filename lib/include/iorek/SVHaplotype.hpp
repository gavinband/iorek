
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_SVHAPLOTYPE_HPP
#define SVELTE_APPS_SVHAPLOTYPE_HPP

#include "iorek/HalfOpenRange.hpp"
#include "iorek/HalfOpenRangeLevel.hpp"
#include "iorek/CoverageProfile.hpp"

namespace iorek {

	// A SVHaplotype conceptually models a finite sequence of integers from some
	// contiguous range e.g. 0, 1, ....., N
	// This is stored as a sequence of contiguous sub-ranges. Operations are
	// provided to compute the actual represented sequence, to compute coverage of
	// each integer, and to splice two haplotypes to produce a new one.
	struct SVHaplotype {
	public:
		SVHaplotype():
			// invariant: m_data is never empty
			m_data( 1, HalfOpenRange( 0, 0 ))
		{}

		SVHaplotype( uint32_t start, uint32_t end ):
			m_data( 1, HalfOpenRange( start, end ))
		{}

		SVHaplotype( SVHaplotype const& other ):
			m_data( other.m_data )
		{}

		SVHaplotype& operator=( SVHaplotype const& other ) {
			m_data = other.m_data ;
			return *this ;
		}

	public:
		// size of this haplotype (considered as a sequence)
		// This is computed by walking the ranges.
		std::size_t size() const ;

		std::size_t number_of_segments() const { return m_data.size() ; }
		// location of first breakpoint. Only valid if number_of_segments() > 1
		std::size_t first_breakpoint() const { return m_data[0].size() ; }
		// location of last breakpoint. Only valid if number_of_segments() > 1
		std::size_t last_breakpoint() const { return size() - m_data.back().size() ; }

		// first position covered by this haplotype
		uint32_t start() const { return m_data.front().start() ; }
		// last position covered by this haplotype
		uint32_t end() const { return m_data.back().end() ; }
		
		friend SVHaplotype splice(
			SVHaplotype const& left,
			uint32_t leftSplicePosition,
			SVHaplotype const& right,
			uint32_t rightSplicePosition
		) ;
		
		CoverageProfile toProfile() const ;
		
		std::vector< uint32_t > toSequence() const ;
		std::string toString() const ;

		friend bool operator==( SVHaplotype const& left, SVHaplotype const& right ) ;
		friend bool operator<( SVHaplotype const& left, SVHaplotype const& right ) ;
	private:
		
		std::vector< HalfOpenRange > m_data ;
	} ;


	inline bool operator==( SVHaplotype const& left, SVHaplotype const& right ) {
		return left.m_data == right.m_data ;
	}

	inline bool operator<( SVHaplotype const& left, SVHaplotype const& right ) {
		return left.m_data < right.m_data ;
	}
}

#endif
