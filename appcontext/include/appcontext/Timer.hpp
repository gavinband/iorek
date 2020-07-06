
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TIMER_HPP
#define TIMER_HPP

#include <string>
#include <chrono>

namespace appcontext {
	struct Timer
	{
		Timer() ;
		Timer( Timer const& other ) ;
		Timer& operator=( Timer const& other ) ;
		double elapsed() const ;
		void restart() ;
		std::string display() const ;
	private:
		std::chrono::time_point< std::chrono::system_clock > m_start ;
	} ;
}

#endif
