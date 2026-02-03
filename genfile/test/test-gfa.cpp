
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "genfile/GFA.hpp"
#include <strstream>

namespace {
	genfile::gfa::GFA::UniquePtr gfa_from_string( std::string data ) {
		std::istringstream s( data ) ;
		return genfile::gfa::GFA::create( s ) ;
	}
}

TEST_CASE( "Parses simple GFA files", "[gfa]" ) {
	typedef std::istringstream S ;
	REQUIRE_NOTHROW( gfa_from_string("") ) ;
	REQUIRE_NOTHROW( gfa_from_string("S\tC1\tACGT\n") ) ;
	REQUIRE_NOTHROW( gfa_from_string("S\tC1\tACGT\nS\tC2\tTGCA\n") ) ;
	REQUIRE_NOTHROW( gfa_from_string("S\tC1\tACGT\nS\tC2\tTGCA\nL\tC1\t+\tC2\t+\t4M\tL1:i:4\tL2:i:0") ) ;
}

TEST_CASE( "Parses correct segments", "[gfa]" ) {
	{
		auto gfa = gfa_from_string( "") ;
		REQUIRE( gfa->number_of_segments() == 0 ) ;
	}

	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\n") ;
		REQUIRE( gfa->number_of_segments() == 1 ) ;
		REQUIRE( gfa->segment(0).name() == "C1" ) ; 
	}

	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\nS\tC2\tTGCA\n") ;
		REQUIRE( gfa->number_of_segments() == 2 ) ;
		REQUIRE( gfa->segment(0).name() == "C1" ) ; 
		REQUIRE( gfa->segment(1).name() == "C2" ) ; 
	}
}

TEST_CASE( "Fails incorrect segments", "[gfa]" ) {
	REQUIRE_THROWS( gfa_from_string( "S" ) ) ;
	REQUIRE_THROWS( gfa_from_string( "S\tC1\n" ) ) ;
	REQUIRE_THROWS( gfa_from_string( "S\tC1\tACGT\nS" ) ) ;
}

TEST_CASE( "Parses correct links", "[gfa]" ) {
	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\nS\tC2\tTGCA\n") ;
		REQUIRE( gfa->number_of_links() == 0 ) ;
	}

	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\nS\tC2\tTGCA\nL\tC1\t+\tC2\t+\t4M\n") ;
		REQUIRE( gfa->number_of_links() == 1 ) ;
		REQUIRE( gfa->link(0).from() == 0 ) ;
		REQUIRE( gfa->link(0).to() == 1 ) ;
		REQUIRE( gfa->link(0).from_orient() == genfile::gfa::ePlus ) ;
		REQUIRE( gfa->link(0).to_orient() == genfile::gfa::ePlus ) ;
		REQUIRE( gfa->link(0).overlap_cigar().has_value() ) ;
		REQUIRE( gfa->link(0).overlap_cigar().get() == "4M" ) ;
	}

	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\nS\tC2\tTGCA\nL\tC1\t-\tC2\t+\t4M\n") ;
		REQUIRE( gfa->number_of_links() == 1 ) ;
		REQUIRE( gfa->link(0).from() == 0 ) ;
		REQUIRE( gfa->link(0).to() == 1 ) ;
		REQUIRE( gfa->link(0).from_orient() == genfile::gfa::eMinus ) ;
		REQUIRE( gfa->link(0).to_orient() == genfile::gfa::ePlus ) ;
		REQUIRE( gfa->link(0).overlap_cigar().has_value() ) ;
		REQUIRE( gfa->link(0).overlap_cigar().get() == "4M" ) ;
	}

	{
		auto gfa = gfa_from_string( "S\tC1\tACGT\nS\tC2\tTGCA\nL\tC1\t-\tC2\t-\t4M\n") ;
		REQUIRE( gfa->number_of_links() == 1 ) ;
		REQUIRE( gfa->link(0).from() == 0 ) ;
		REQUIRE( gfa->link(0).to() == 1 ) ;
		REQUIRE( gfa->link(0).from_orient() == genfile::gfa::eMinus ) ;
		REQUIRE( gfa->link(0).to_orient() == genfile::gfa::eMinus ) ;
		REQUIRE( gfa->link(0).overlap_cigar().has_value() ) ;
		REQUIRE( gfa->link(0).overlap_cigar().get() == "4M" ) ;
	}
}

TEST_CASE( "Fails incorrect links", "[gfa]" ) {
	REQUIRE_THROWS( gfa_from_string( "S\tC1\tACGT\nL\tC1\t+\tC2\t+\t4M\n")) ;
	REQUIRE_THROWS( gfa_from_string( "S\tC1\tACGT\nL\tC1\ta\tC1\t+\t4M\n")) ;
	REQUIRE_THROWS( gfa_from_string( "S\tC1\tACGT\nL\tC1\t+\tC1\ta\t4M\n")) ;
	REQUIRE_THROWS( gfa_from_string( "S\tC1\tACGT\nL\tC1\t+\tC1\t+\n")) ;
}
