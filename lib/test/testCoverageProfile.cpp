
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <sstream>
#include "svelte/CoverageProfile.hpp"

#include "catch.hpp"

using namespace svelte ;

TEST_CASE( "CoverageProfile:construction" ) {
	REQUIRE( CoverageProfile(0,0,0).toString() == "[0,0):0" ) ;
	REQUIRE( CoverageProfile( 0, 1, 0 ).toString() == "[0,1):0" ) ;
	REQUIRE( CoverageProfile( 0, 4, 0 ).toString() == "[0,4):0" ) ;
	REQUIRE( CoverageProfile( 1, 4, 0 ).toString() == "[1,4):0" ) ;
	REQUIRE( CoverageProfile( 0, 0, 1 ).toString() == "[0,0):1" ) ;
	REQUIRE( CoverageProfile( 0, 4, 1 ).toString() == "[0,4):1" ) ;
	REQUIRE( CoverageProfile( 1, 4, 1 ).toString() == "[1,4):1" ) ;
}

TEST_CASE( "CoverageProfile:addition" ) {
	
	SECTION( "Can add to single segment profile" ) {
		CoverageProfile profile( 1, 3, 1 ) ;

		// add zero-width profile
		REQUIRE( (profile + CoverageProfile( 0, 0, 1 )).toString() == "[1,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 1, 1 )).toString() == "[1,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 2, 1 )).toString() == "[1,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 3, 3, 1 )).toString() == "[1,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 4, 4, 1 )).toString() == "[1,3):1" ) ;

		// add one-width profile
		REQUIRE( (profile + CoverageProfile( 0, 1, 1 )).toString() == "[1,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 2, 1 )).toString() == "[1,2):2;[2,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 3, 1 )).toString() == "[1,2):1;[2,3):2" ) ;
		REQUIRE( (profile + CoverageProfile( 3, 4, 1 )).toString() == "[1,3):1" ) ;

		// add 2-width profiles
		REQUIRE( (profile + CoverageProfile( 0, 2, 1 )).toString() == "[1,2):2;[2,3):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 3, 1 )).toString() == "[1,3):2" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 4, 1 )).toString() == "[1,2):1;[2,3):2" ) ;

		// add longer profiles
		REQUIRE( (profile + CoverageProfile( 0, 3, 1 )).toString() == "[1,3):2" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 4, 1 )).toString() == "[1,3):2" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 5, 1 )).toString() == "[1,2):1;[2,3):2" ) ;
	}

	SECTION( "Can add to multiple segment profile" ) {
		CoverageProfile profile( 1, 4, 1 ) ;
		profile += CoverageProfile( 2, 3, 1 ) ;

		// add 0-length profile
		REQUIRE( (profile + CoverageProfile( 0, 0, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 1, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 2, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 3, 3, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 4, 4, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;

		// add 1-length profile
		REQUIRE( (profile + CoverageProfile( 0, 1, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 2, 1 )).toString() == "[1,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 3, 1 )).toString() == "[1,2):1;[2,3):3;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 3, 4, 1 )).toString() == "[1,2):1;[2,4):2" ) ;
		REQUIRE( (profile + CoverageProfile( 4, 5, 1 )).toString() == "[1,2):1;[2,3):2;[3,4):1" ) ;

		// add 2-length profile
		REQUIRE( (profile + CoverageProfile( 0, 2, 1 )).toString() == "[1,3):2;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 1, 3, 1 )).toString() == "[1,2):2;[2,3):3;[3,4):1" ) ;
		REQUIRE( (profile + CoverageProfile( 2, 4, 1 )).toString() == "[1,2):1;[2,3):3;[3,4):2" ) ;
		REQUIRE( (profile + CoverageProfile( 3, 5, 1 )).toString() == "[1,2):1;[2,4):2" ) ;
	}
}

