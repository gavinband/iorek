
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_HALFOPENRANGE_HPP
#define SVELTE_APPS_HALFOPENRANGE_HPP

#include <cassert>

namespace iorek {
	
	// model a half-open-range [start,end)]
	struct HalfOpenRange {
		typedef uint32_t Integer ;
		HalfOpenRange():
			m_start(0),
			m_end(0)
		{}

		HalfOpenRange( Integer start, Integer end ):
			m_start( start ),
			m_end( end )
		{
			assert( m_end >= m_start ) ;
		}
		
		HalfOpenRange( HalfOpenRange const& other ):
			m_start( other.m_start ),
			m_end( other.m_end )
		{}
		
		HalfOpenRange& operator=( HalfOpenRange const& other ) {
			m_start = other.m_start ;
			m_end = other.m_end ;
			return *this ;
		}

		Integer start() const { return m_start ; }
		Integer end() const { return m_end ; }
		Integer size() const { return m_end - m_start ; }

		friend bool operator==( HalfOpenRange const& left, HalfOpenRange const& right ) ;
		friend bool operator<( HalfOpenRange const& left, HalfOpenRange const& right ) ;
	private:
		Integer m_start ;
		Integer m_end ;
	} ;

	inline bool operator==( HalfOpenRange const& left, HalfOpenRange const& right ) {
		return (left.m_start == right.m_start) && ( left.m_end == right.m_end ) ;
	}
	
	inline bool operator<( HalfOpenRange const& left, HalfOpenRange const& right ) {
		return (left.m_start < right.m_start) || ( left.m_start == right.m_start && left.m_end < right.m_end ) ;
	}

}

#endif
