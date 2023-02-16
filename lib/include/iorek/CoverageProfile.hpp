
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_COVERAGEPROFILE_HPP
#define SVELTE_APPS_COVERAGEPROFILE_HPP

#include <vector>
#include <string>
#include "iorek/HalfOpenRangeLevel.hpp"

namespace iorek {
	struct CoverageProfile {
	public:
		CoverageProfile():
			m_data( 1, HalfOpenRangeLevel( HalfOpenRange(), 0 ))
		{
			// invariant: m_data is never empty.
		}

		CoverageProfile( uint32_t start, uint32_t end, uint32_t level ):
			m_data( 1, HalfOpenRangeLevel( start, end, level ))
		{}

		CoverageProfile( CoverageProfile const& other  ):
			m_data( other.m_data )
		{}

		CoverageProfile& operator=( CoverageProfile const& other  ) {
			m_data = other.m_data ;
			return *this ;
		}

		std::string toString() const ;
		std::string prettyPrint() const ;

	public:
		std::size_t size() const { return m_data.size() ; }
		HalfOpenRangeLevel const* begin() const { return (&m_data[0]) ; }
		HalfOpenRangeLevel const* const end() const { return (&m_data[0]) + m_data.size() ; }

		CoverageProfile& operator+=( HalfOpenRangeLevel const& other ) ;
		CoverageProfile& operator+=( CoverageProfile const& other ) ;

		friend CoverageProfile operator+( CoverageProfile const& left, CoverageProfile const& right ) ;
		friend bool operator==( CoverageProfile const& left, CoverageProfile const& right ) ;
		friend bool operator<( CoverageProfile const& left, CoverageProfile const& right ) ;
	private:
		std::vector< HalfOpenRangeLevel > m_data ;
	} ;
	
	inline bool operator==( CoverageProfile const& left, CoverageProfile const& right ) {
		return left.m_data == right.m_data ;
	}

	inline bool operator<( CoverageProfile const& left, CoverageProfile const& right ) {
		return left.m_data < right.m_data ;
	}
}

#endif
