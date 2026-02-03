//					Copyright Gavin Band 2008 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOREK_LOG_SUM_EXP_HPP
#define IOREK_LOG_SUM_EXP_HPP

#include <cassert>

namespace iorek {
	namespace {
		template< typename Iterator, typename F >
		F log_sum_exp( Iterator begin, Iterator const& end, F const max_value ) {
				F running_sum = 0.0 ;
				for( ; begin != end; ++begin ) {
						running_sum += std::exp( *begin - max_value ) ;
				}
				return max_value + std::log( running_sum ) ;
		}
	}

	template< typename Iterator >
	typename std::iterator_traits< Iterator >::value_type
	log_sum_exp( Iterator const& begin, Iterator const& end ) {
		typedef typename std::iterator_traits< Iterator >::value_type F ;
		F max_value = -std::numeric_limits< F >::infinity() ;
		for( Iterator i = begin; i != end; ++i ) {
				max_value = std::max( max_value, *i ) ;
		}
		if( max_value == -std::numeric_limits< F >::infinity() ) {
				return -std::numeric_limits< F >::infinity() ;
		}
		else {
			return log_sum_exp( begin, end, max_value ) ;
		}
	}
}

#endif
