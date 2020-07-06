
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_RANGECOVERAGE_HPP
#define SVELTE_APPS_RANGECOVERAGE_HPP

#include <vector>
#include "svelte/HalfOpenRange.hpp"

namespace svelte {
	struct HalfOpenRangeLevel {
		HalfOpenRangeLevel():
			m_range(),
			m_level(0)
		{}

		HalfOpenRangeLevel(
			HalfOpenRange const& range,
			uint32_t const& level
		):
			m_range( range ),
			m_level( level )
		{}

		HalfOpenRangeLevel(
			uint32_t const start,
			uint32_t const end,
			uint32_t const& level
		):
			m_range( start, end ),
			m_level( level )
		{}

		HalfOpenRangeLevel( HalfOpenRangeLevel const& other ):
			m_range( other.m_range ),
			m_level( other.m_level )
		{}

		HalfOpenRangeLevel& operator=( HalfOpenRangeLevel const& other ) {
			m_range = other.m_range ;
			m_level = other.m_level ;
			return *this ;
		}

		uint32_t start() const { return m_range.start() ; }
		uint32_t end() const { return m_range.end() ; }
		uint32_t size() const { return m_range.size() ; }
		uint32_t level() const { return m_level ; }
		
		friend bool operator==( HalfOpenRangeLevel const& left, HalfOpenRangeLevel const& right ) ;
		friend bool operator<( HalfOpenRangeLevel const& left, HalfOpenRangeLevel const& right ) ;
	private:
		HalfOpenRange m_range ;
		uint32_t m_level ;
	} ;

	inline bool operator==( HalfOpenRangeLevel const& left, HalfOpenRangeLevel const& right ) {
		return left.m_range == right.m_range && left.m_level == right.m_level ;
	}

	inline bool operator<( HalfOpenRangeLevel const& left, HalfOpenRangeLevel const& right ) {
		return (left.m_range < right.m_range) || (left.m_range == right.m_range && left.m_level < right.m_level ) ;
	}
}

#endif
