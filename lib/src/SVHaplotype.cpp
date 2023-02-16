
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include "iorek/HalfOpenRange.hpp"
#include "iorek/HalfOpenRangeLevel.hpp"
#include "iorek/SVHaplotype.hpp"

namespace iorek {
	// size of this haplotype (considered as a sequence)
	std::size_t SVHaplotype::size() const {
		std::size_t result = 0 ;
		for( auto const& range: m_data ) {
			result += range.size() ;
		}
		return result ;
	}
		
	CoverageProfile SVHaplotype::toProfile() const {
		CoverageProfile result( start(), end(), 0 ) ;
		for( auto const& range: m_data ) {
			result += HalfOpenRangeLevel( range, 1 ) ;
		}
		return result ;
	}
	
	std::string SVHaplotype::toString() const {
		std::ostringstream str ;
		std::size_t count = 0 ;
		for( auto const& range: m_data ) {
			for( uint32_t i = range.start(); i < range.end(); ++i, ++count ) {
				str << ((count>0) ? " " : "" ) << i ;
			}
		}
		return str.str() ;
	}

	std::vector< uint32_t > SVHaplotype::toSequence() const {
		std::vector< uint32_t > result ;
		result.reserve( this->size() ) ;
		for( auto const& range: m_data ) {
			for( uint32_t i = range.start(); i < range.end(); ++i ) {
				result.push_back( i ) ;
			}
		}
		return result ;
	}

	SVHaplotype splice(
		SVHaplotype const& left,
		uint32_t leftSplicePosition,
		SVHaplotype const& right,
		uint32_t rightSplicePosition
	) {
		SVHaplotype result ;
		std::size_t i = 0 ;
		std::size_t leftLengthCovered = left.m_data[0].size() ;
		// copy ranges from left haplotype
		for( ; leftLengthCovered <= leftSplicePosition; leftLengthCovered += left.m_data[++i].size() ) {
			result.m_data.push_back( left.m_data[i] ) ;
		}

		// find right interval to split
		std::size_t j = 0 ;
		
		std::size_t rightLengthCovered = right.m_data[0].size() ;
		for( ; rightLengthCovered < rightSplicePosition; rightLengthCovered += right.m_data[++j].size() ) {
			// nothing to do
		}

		// find the needed info for the joining pieces
		uint32_t leftStart = left.m_data[i].start() ;
		uint32_t atLeftPosition = left.m_data[i].end() + (leftSplicePosition - leftLengthCovered) ;
		uint32_t atRightPosition = right.m_data[j].end() + (rightSplicePosition - rightLengthCovered) ;
		uint32_t rightEnd = right.m_data[j].end() ;

		if( atLeftPosition == atRightPosition && rightEnd > leftStart ) {
			result.m_data.push_back( HalfOpenRange( leftStart, rightEnd )) ;
		} else {
			if( atLeftPosition > leftStart ) {
				result.m_data.push_back( HalfOpenRange( leftStart, atLeftPosition )) ;
			}
			if( rightEnd > atRightPosition ) {
				result.m_data.push_back( HalfOpenRange( atRightPosition, rightEnd )) ;
			}
		}

		// find right interval to split
		for( ++j; j < right.m_data.size(); ++j ) {
			result.m_data.push_back( right.m_data[j] ) ;
		}

		return result ;
	}
}

