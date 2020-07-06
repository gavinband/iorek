
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef STATFILE_TEST_CASE_HPP
#define STATFILE_TEST_CASE_HPP

#include <cassert>
#include "config/config.hpp"

#if HAVE_BOOST_UNIT_TEST_FRAMEWORK
	#include "boost/test/auto_unit_test.hpp"
	#define AUTO_TEST_CASE( param ) BOOST_AUTO_TEST_CASE(param)
	#define TEST_ASSERT( param ) BOOST_ASSERT( param )
	#define AUTO_TEST_MAIN void test_case_dummy_function_WILL_NOT_BE_CALLED()
#else
	#define AUTO_TEST_CASE( param ) void param()
	#define TEST_ASSERT( param ) assert( param )
	#define AUTO_TEST_MAIN int main( int argc, char** argv )
#endif	

#endif
