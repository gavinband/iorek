
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "genfile/repeats/repeat_tracts.hpp"
#include "genfile/repeats/HomopolymerTractWalker.hpp"

TEST_CASE( "repeats are identified" ) {
	{
		std::string s = "TGAGAGAGT" ;
		std::string::iterator start( s.begin() ) ;
		std::string::iterator end( s.begin() + 1 ) ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == s.begin() ) ;
		REQUIRE( end == s.begin() + 1 ) ;
		
		start = s.begin() + 1 ;
		end = s.begin() + 3 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;

		start = s.begin() + 2 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;

		start = s.begin() + 0 ;
		end = s.begin() + 2 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 0) ) ;
		REQUIRE( end == (s.begin() + 2) ) ;

		start = s.begin() + 1 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 4) ) ;

		start = s.begin() + 2 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;
	}
}

TEST_CASE( "repeats are identified to the left" ) {
	{
		std::string s = "TGAGAGAGT" ;
		std::string::iterator start( s.begin() ) ;
		std::string::iterator end( s.begin() + 1 ) ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == s.begin() ) ;
		REQUIRE( end == s.begin() + 1 ) ;
		
		start = s.begin() + 1 ;
		end = s.begin() + 3 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;

		start = s.begin() + 2 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;

		start = s.begin() + 0 ;
		end = s.begin() + 2 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 0) ) ;
		REQUIRE( end == (s.begin() + 2) ) ;

		start = s.begin() + 1 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 4) ) ;

		start = s.begin() + 2 ;
		end = s.begin() + 4 ;
		genfile::repeats::expand( s.begin(), s.end(), start, end ) ;
		std::cerr << ":" << std::string( start, end ) << ".\n" ;
		REQUIRE( start == (s.begin() + 1) ) ;
		REQUIRE( end == (s.begin() + 8) ) ;
	}
}

TEST_CASE( "Can walk tracts" ) {
	typedef std::string str ;
	str sequence = "ACGTAAGTTCCGGGGA" ; // length 16
	str::iterator p = sequence.begin() ;
	genfile::repeats::HomopolymerTractWalker< str::iterator > w( p, sequence.end(), p ) ;
	
	REQUIRE( w.position() == p ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "A" ) ;

	++w ;
	REQUIRE( w.position() == p+1 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "A" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "C" ) ;

	++w ;
	REQUIRE( w.position() == p+2 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "C" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "G" ) ;

	++w ;
	REQUIRE( w.position() == p+3 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "G" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "T" ) ;

	++w ;
	REQUIRE( w.position() == p+4 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "T" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "AA" ) ;

	++w ;
	REQUIRE( w.position() == p+5 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "AA" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "AA" ) ;

	++w ;
	REQUIRE( w.position() == p+6 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "AA" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "G" ) ;

	++w ;
	REQUIRE( w.position() == p+7 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "G" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "TT" ) ;

	++w ;
	REQUIRE( w.position() == p+8 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "TT" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "TT" ) ;

	++w ;
	REQUIRE( w.position() == p+9 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "TT" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "CC" ) ;

	++w ;
	REQUIRE( w.position() == p+10 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "CC" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "CC" ) ;

	//	std::string sequence = "ACGTAAGTTCCGGGGA" ;
	++w ;
	REQUIRE( w.position() == p+11 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "CC" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "GGGG" ) ;

	++w ;
	REQUIRE( w.position() == p+12 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "GGGG" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "GGGG" ) ;

	++w ;
	REQUIRE( w.position() == p+13 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "GGGG" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "GGGG" ) ;

	++w ;
	REQUIRE( w.position() == p+14 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "GGGG" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "GGGG" ) ;

	++w ;
	REQUIRE( w.position() == p+15 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "GGGG" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "A" ) ;

	++w ;
	REQUIRE( w.position() == p+16 ) ;
	REQUIRE( str( w.left_tract().begin(), w.left_tract().end() ) == "A" ) ;
	REQUIRE( str( w.right_tract().begin(), w.right_tract().end() ) == "" ) ;

	REQUIRE( w.position() == w.contig_end() ) ;
	
}

