
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <sstream>
#include "iorek/SVPopulationModel.hpp"

#include "catch.hpp"

using namespace iorek ;

TEST_CASE( "SVPopulationModel:construction" ) {
	REQUIRE( SVPopulationModel(0,0).toString() == "{[0,0):1:1}" ) ;
	REQUIRE( SVPopulationModel(0,1).toString() == "{[0,1):1:1}" ) ;
	REQUIRE( SVPopulationModel(0,5).toString() == "{[0,5):1:1}" ) ;
	REQUIRE( SVPopulationModel(2,4).toString() == "{[2,4):1:1}" ) ;

	// test copy constructor
	auto model = SVPopulationModel(0,1) ;
	REQUIRE( SVPopulationModel( model ).toString() == "{[0,1):1:1}" ) ;

	// test assignment
	SVPopulationModel model2( 0, 10 ) ;
	model2 = model ;
	REQUIRE( SVPopulationModel( model2 ).toString() == "{[0,1):1:1}" ) ;
}

TEST_CASE( "SVPopulationModel:add" ) {
	SVPopulationModel model(0,10) ;
	model.add( CoverageProfile( 0, 10, 2 ), 0.01 ) ;
	REQUIRE( model.toString() == "{[0,10):1:0.99|[0,10):2:0.01}" ) ;
	model.add( CoverageProfile( 0, 10, 3 ), 0.01 ) ;
	REQUIRE( model.toString() == "{[0,10):1:0.9801|[0,10):2:0.0099|[0,10):3:0.01}" ) ;
}

TEST_CASE( "SVPopulationModel:remove" ) {
	SVPopulationModel model(0,10) ;
	model.add( CoverageProfile( 0, 10, 2 ), 0.01 ) ;
	model.remove( CoverageProfile( 0, 10, 2 ) ) ;
	REQUIRE( model.toString() == "{[0,10):1:1}" ) ;

	model.add( CoverageProfile( 0, 10, 2 ), 0.01 ) ; // {[0,10):1:0.99,[0,10):2:0.01}
	model.add( CoverageProfile( 0, 10, 3 ), 0.01 ) ; // {[0,10):1:0.9801,[0,10):2:0.0099,[0,10):3:0.01}
	model.remove( CoverageProfile( 0, 10, 2 ) ) ;
	REQUIRE( model.toString() == "{[0,10):1:0.9899|[0,10):3:0.0101}" ) ;
}
