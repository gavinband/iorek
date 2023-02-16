
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <sstream>
#include "iorek/SVHaplotype.hpp"

#include "catch.hpp"

using namespace iorek ;

TEST_CASE( "SVHaplotype:construction" ) {
	REQUIRE( SVHaplotype().toString() == "" ) ;
	REQUIRE( SVHaplotype(0, 1).toString() == "0" ) ;
	REQUIRE( SVHaplotype(0, 2).toString() == "0 1" ) ;
	REQUIRE( SVHaplotype(1, 2).toString() == "1" ) ;
	REQUIRE( SVHaplotype(1, 8).toString() == "1 2 3 4 5 6 7" ) ;
	REQUIRE( SVHaplotype(7, 12).toString() == "7 8 9 10 11" ) ;
	REQUIRE( SVHaplotype(10, 12).toString() == "10 11" ) ;
}

TEST_CASE( "SVHaplotype:splice" ) {
	SVHaplotype hap( 0, 4 ) ;
	REQUIRE( hap.toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 0, hap, 0 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 0, hap, 1 ).toString() == "1 2 3" ) ;
	REQUIRE( splice( hap, 0, hap, 2 ).toString() == "2 3" ) ;
	REQUIRE( splice( hap, 0, hap, 3 ).toString() == "3" ) ;
	REQUIRE( splice( hap, 0, hap, 4 ).toString() == "" ) ;

	REQUIRE( splice( hap, 1, hap, 0 ).toString() == "0 0 1 2 3" ) ;
	REQUIRE( splice( hap, 1, hap, 1 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 1, hap, 2 ).toString() == "0 2 3" ) ;
	REQUIRE( splice( hap, 1, hap, 3 ).toString() == "0 3" ) ;
	REQUIRE( splice( hap, 1, hap, 4 ).toString() == "0" ) ;

	REQUIRE( splice( hap, 2, hap, 0 ).toString() == "0 1 0 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap, 1 ).toString() == "0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap, 2 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap, 3 ).toString() == "0 1 3" ) ;
	REQUIRE( splice( hap, 2, hap, 4 ).toString() == "0 1" ) ;

	REQUIRE( splice( hap, 3, hap, 0 ).toString() == "0 1 2 0 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap, 1 ).toString() == "0 1 2 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap, 2 ).toString() == "0 1 2 2 3" ) ;
	REQUIRE( splice( hap, 3, hap, 3 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap, 4 ).toString() == "0 1 2" ) ;

	REQUIRE( splice( hap, 4, hap, 0 ).toString() == "0 1 2 3 0 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap, 1 ).toString() == "0 1 2 3 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap, 2 ).toString() == "0 1 2 3 2 3" ) ;
	REQUIRE( splice( hap, 4, hap, 3 ).toString() == "0 1 2 3 3" ) ;
	REQUIRE( splice( hap, 4, hap, 4 ).toString() == "0 1 2 3" ) ;

	SVHaplotype hap2( 10, 12 ) ;
	REQUIRE( hap2.toString() == "10 11" ) ;
	REQUIRE( splice( hap, 0, hap2, 0 ).toString() == "10 11" ) ;
	REQUIRE( splice( hap, 1, hap2, 0 ).toString() == "0 10 11" ) ;
	REQUIRE( splice( hap, 2, hap2, 0 ).toString() == "0 1 10 11" ) ;
	REQUIRE( splice( hap, 3, hap2, 0 ).toString() == "0 1 2 10 11" ) ;
	REQUIRE( splice( hap, 4, hap2, 0 ).toString() == "0 1 2 3 10 11" ) ;

	REQUIRE( splice( hap, 0, hap2, 1 ).toString() == "11" ) ;
	REQUIRE( splice( hap, 1, hap2, 1 ).toString() == "0 11" ) ;
	REQUIRE( splice( hap, 2, hap2, 1 ).toString() == "0 1 11" ) ;
	REQUIRE( splice( hap, 3, hap2, 1 ).toString() == "0 1 2 11" ) ;
	REQUIRE( splice( hap, 4, hap2, 1 ).toString() == "0 1 2 3 11" ) ;

	REQUIRE( splice( hap, 0, hap2, 2 ).toString() == "" ) ;
	REQUIRE( splice( hap, 1, hap2, 2 ).toString() == "0" ) ;
	REQUIRE( splice( hap, 2, hap2, 2 ).toString() == "0 1" ) ;
	REQUIRE( splice( hap, 3, hap2, 2 ).toString() == "0 1 2" ) ;
	REQUIRE( splice( hap, 4, hap2, 2 ).toString() == "0 1 2 3" ) ;
	
	SVHaplotype hap3 = splice( hap, 2, hap, 1 ) ; // 0 1 1 2 3
	REQUIRE( splice( hap, 0, hap3, 0 ).toString() == "0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 1, hap3, 0 ).toString() == "0 0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap3, 0 ).toString() == "0 1 0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap3, 0 ).toString() == "0 1 2 0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap3, 0 ).toString() == "0 1 2 3 0 1 1 2 3" ) ;

	REQUIRE( splice( hap, 0, hap3, 1 ).toString() == "1 1 2 3" ) ;
	REQUIRE( splice( hap, 1, hap3, 1 ).toString() == "0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap3, 1 ).toString() == "0 1 1 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap3, 1 ).toString() == "0 1 2 1 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap3, 1 ).toString() == "0 1 2 3 1 1 2 3" ) ;

	REQUIRE( splice( hap, 0, hap3, 2 ).toString() == "1 2 3" ) ;
	REQUIRE( splice( hap, 1, hap3, 2 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 2, hap3, 2 ).toString() == "0 1 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap3, 2 ).toString() == "0 1 2 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap3, 2 ).toString() == "0 1 2 3 1 2 3" ) ;

	REQUIRE( splice( hap, 0, hap3, 3 ).toString() == "2 3" ) ;
	REQUIRE( splice( hap, 1, hap3, 3 ).toString() == "0 2 3" ) ;
	REQUIRE( splice( hap, 2, hap3, 3 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 3, hap3, 3 ).toString() == "0 1 2 2 3" ) ;
	REQUIRE( splice( hap, 4, hap3, 3 ).toString() == "0 1 2 3 2 3" ) ;

	REQUIRE( splice( hap, 0, hap3, 4 ).toString() == "3" ) ;
	REQUIRE( splice( hap, 1, hap3, 4 ).toString() == "0 3" ) ;
	REQUIRE( splice( hap, 2, hap3, 4 ).toString() == "0 1 3" ) ;
	REQUIRE( splice( hap, 3, hap3, 4 ).toString() == "0 1 2 3" ) ;
	REQUIRE( splice( hap, 4, hap3, 4 ).toString() == "0 1 2 3 3" ) ;

	REQUIRE( splice( hap, 0, hap3, 5 ).toString() == "" ) ;
	REQUIRE( splice( hap, 1, hap3, 5 ).toString() == "0" ) ;
	REQUIRE( splice( hap, 2, hap3, 5 ).toString() == "0 1" ) ;
	REQUIRE( splice( hap, 3, hap3, 5 ).toString() == "0 1 2" ) ;
	REQUIRE( splice( hap, 4, hap3, 5 ).toString() == "0 1 2 3" ) ;
	
	// DUP4 formation example
	SVHaplotype hap4( 0, 5 ) ;
	SVHaplotype hap5 = splice( hap4, 2, hap4, 1 ) ;
	REQUIRE( hap5.toString() == "0 1 1 2 3 4" ) ;
	REQUIRE( splice( hap5, 5, hap5, 4 ).toString() == "0 1 1 2 3 3 4" ) ;
}

TEST_CASE( "SVHaplotype:size" ) {
	SVHaplotype hap( 0, 4 ) ;
	REQUIRE( hap.size() == 4 ) ;
	REQUIRE( splice( hap, 2, hap, 1 ).size() == 5 ) ;
	REQUIRE( splice( hap, 1, hap, 2 ).size() == 3 ) ;
	REQUIRE( splice( hap, 4, hap, 0 ).size() == 8 ) ;
	REQUIRE( splice( hap, 0, hap, 4 ).size() == 0 ) ;
}

TEST_CASE( "SVHaplotype:toProfile" ) {
	SVHaplotype hap( 0, 4 ) ;
	REQUIRE( hap.toString() == "0 1 2 3" ) ;
	REQUIRE( hap.toProfile().toString() == "[0,4):1" ) ;
	REQUIRE( splice( hap, 2, hap, 1 ).toProfile().toString() == "[0,1):1;[1,2):2;[2,4):1" ) ;
	REQUIRE( splice( hap, 3, hap, 1 ).toProfile().toString() == "[0,1):1;[1,3):2;[3,4):1" ) ;
	REQUIRE( splice( hap, 1, hap, 2 ).toProfile().toString() == "[0,1):1;[1,2):0;[2,4):1" ) ;
	REQUIRE( splice( hap, 1, hap, 3 ).toProfile().toString() == "[0,1):1;[1,3):0;[3,4):1" ) ;
}
