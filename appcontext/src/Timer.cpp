
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "appcontext/Timer.hpp"

using std::chrono::time_point ;
using std::chrono::system_clock ;
using std::chrono::duration ;

namespace appcontext {
	Timer::Timer():
		m_start( system_clock::now() )
	{}

	Timer::Timer( Timer const& other ):
		m_start( other.m_start )
	{}

	Timer& Timer::operator=( Timer const& other ) {
		m_start = other.m_start ;
		return *this ;
	}

	double Timer::elapsed() const {
		time_point<system_clock> now = system_clock::now() ;
		duration< double > elapsed = now - m_start ;
		return elapsed.count() ;
	}
	
	void Timer::restart() {
		m_start = system_clock::now() ;
	}
	
	std::string Timer::display() const {
		std::ostringstream os ;
		os << std::fixed << std::setprecision(1) << elapsed() << "s" ;
		return os.str() ;
	}
}
