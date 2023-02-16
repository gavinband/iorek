
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "iorek/HalfOpenRangeLevel.hpp"
#include "iorek/CoverageProfile.hpp"

namespace iorek {
	std::string CoverageProfile::toString() const {
		std::ostringstream result ;
		for( std::size_t i = 0; i < m_data.size(); ++i ) {
			auto range = m_data[i] ;
			result
				<< ((i>0) ? ";" : "")
				<< "[" << range.start() << "," << range.end() << "):" << range.level() ;
		}
		return result.str() ;
	}

	std::string CoverageProfile::prettyPrint() const {
		std::ostringstream result ;
		for( std::size_t i = 0; i < m_data.size(); ++i ) {
			auto range = m_data[i] ;
			result
				<< ((i>0) ? "; " : "")
				<< "[" << std::setw(4) << range.start() << "," << std::setw(4) << range.end() << "): " << range.level() ;
		}
		return result.str() ;
	}

	CoverageProfile& CoverageProfile::operator+=( HalfOpenRangeLevel const& other ) {
		std::vector< HalfOpenRangeLevel > new_data ;
		new_data.reserve( m_data.size() + 3 ) ;

		auto addIfNotEmpty = [&new_data](
			uint32_t const start,
			uint32_t const end,
			uint32_t const level
		) {
			if( end > start ) {
				new_data.push_back( HalfOpenRangeLevel( start, end, level )) ;
			}
		} ;

		for( auto range: m_data ) {
			if( range.end() <= other.start() || range.start() >= other.end() ) {
				new_data.push_back( range ) ;
			} else {
				// range intersects.
				// We have to add one, two or three pieces according
				// to whether they overlap at both, one, or neither end.
				if( other.start() > range.start() ) {
					addIfNotEmpty(
						range.start(),
						other.start(),
						range.level()
					) ;
				}
				
				addIfNotEmpty(
					std::max( range.start(), other.start() ),
					std::min( range.end(), other.end() ),
					range.level() + other.level()
				) ;
					
				if( other.end() < range.end() ) {
					addIfNotEmpty(
						other.end(),
						range.end(),
						range.level()
					) ;
				}
			}
		}
		
		// Merge adjacent ranges
		{
			typedef std::vector< HalfOpenRangeLevel >::iterator Iterator ;
			assert( new_data.size() > 0 ) ;
			Iterator j = new_data.begin() ;
			// this loop mutates and iterates at the same time
			for( Iterator i = j; (++j) != new_data.end(); i = j ) {
				if( i->level() == j->level() ) {
					*i = HalfOpenRangeLevel( i->start(), j->end(), i->level() ) ;
					new_data.erase( j ) ;
					j = i ;
				}
			}
		}
		
		m_data = new_data ;
		return *this ;
	}

	CoverageProfile& CoverageProfile::operator+=( CoverageProfile const& other ) {
		// TODO: Fix this implementation to be less inefficient (this will lead to
		// multiple allocations of new data vector).
		for( auto range: other ) {
			(*this) += range ;
		}
		return *this ;
	}

	CoverageProfile operator+( CoverageProfile const& left, CoverageProfile const& right ) {
		CoverageProfile result = left ;
		result += right ;
		return result ;
	}
}

