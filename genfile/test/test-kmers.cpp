
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "genfile/kmer/KmerHashIterator.hpp"

TEST_CASE( "1mer hashes are computed", "[kmer]" ) {
	std::string str( "ACGTAACCGGTTAAACCCGGGTTT" ) ;
	genfile::kmer::KmerHashIterator< std::string::const_iterator > it( str.begin(), str.end(), 1 ) ;
	REQUIRE( it.hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( !it.finished() ) ;

	REQUIRE( (++it).hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( !it.finished() ) ;

	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( !it.finished() ) ;

	REQUIRE( (++it).hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 0 ) ;
	REQUIRE( it.reverse_complement_hash() == 3 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( (++it).hash() == 1 ) ;
	REQUIRE( it.reverse_complement_hash() == 2 ) ;
	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 2 ) ;
	REQUIRE( it.reverse_complement_hash() == 1 ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( !it.finished() ) ;
	REQUIRE( (++it).hash() == 3 ) ;
	REQUIRE( it.reverse_complement_hash() == 0 ) ;
	REQUIRE( it.finished() ) ;
}

TEST_CASE( "2mer hashes are computed", "[kmer]" ) {
	std::string str( "ACGTGCAACCGGTT" ) ;
	genfile::kmer::KmerHashIterator< std::string::const_iterator > it( str.begin(), str.end(), 2 ) ;

	// AC, CG, GT
	REQUIRE( it.hash() == 0x1 ) ;
	REQUIRE( it.reverse_complement_hash() == 0xB ) ;
	REQUIRE( (++it).hash() == 0x6 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x6 ) ;
	REQUIRE( (++it).hash() == 0xB ) ;
	REQUIRE( it.reverse_complement_hash() == 0x1 ) ;
	REQUIRE( !it.finished() ) ;

	// TG, GC, CA
	REQUIRE( (++it).hash() == 0xE ) ;
	REQUIRE( it.reverse_complement_hash() == 0x4 ) ;
	REQUIRE( (++it).hash() == 0x9 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x9 ) ;
	REQUIRE( (++it).hash() == 0x4 ) ;
	REQUIRE( it.reverse_complement_hash() == 0xE ) ;
	REQUIRE( !it.finished() ) ;

	// AA, AC, CC, CG, GG, 
	REQUIRE( (++it).hash() == 0x0 ) ;
	REQUIRE( it.reverse_complement_hash() == 0xF ) ;
	REQUIRE( (++it).hash() == 0x1 ) ;
	REQUIRE( it.reverse_complement_hash() == 0xB ) ;
	REQUIRE( (++it).hash() == 0x5 ) ;
	REQUIRE( it.reverse_complement_hash() == 0xA ) ;
	REQUIRE( (++it).hash() == 0x6 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x6 ) ;
	REQUIRE( (++it).hash() == 0xA ) ;
	REQUIRE( it.reverse_complement_hash() == 0x5 ) ;
	REQUIRE( !it.finished() ) ;
	// GT, TT
	REQUIRE( (++it).hash() == 0xB ) ;
	REQUIRE( it.reverse_complement_hash() == 0x1 ) ;
	REQUIRE( !it.finished() ) ;
	REQUIRE( (++it).hash() == 0xF ) ;
	REQUIRE( it.reverse_complement_hash() == 0x0 ) ;
	REQUIRE( it.finished() ) ;

}

TEST_CASE( "3mer hashes are computed", "[kmer]" ) {
	std::string str( "ACGTGCA" ) ;
	genfile::kmer::KmerHashIterator< std::string::const_iterator > it( str.begin(), str.end(), 3 ) ;

	// ACG, CGT, GTG
	REQUIRE( it.hash() == 0x06 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x1B ) ;
	REQUIRE( (++it).hash() == 0x1B ) ;
	REQUIRE( it.reverse_complement_hash() == 0x06 ) ;
	REQUIRE( (++it).hash() == 0x2E ) ;
	REQUIRE( it.reverse_complement_hash() == 0x11 ) ;
	REQUIRE( !it.finished() ) ;

	// TGC, GCA
	REQUIRE( (++it).hash() == 0x39 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x24 ) ;
	REQUIRE( !it.finished() ) ;
	REQUIRE( (++it).hash() == 0x24 ) ;
	REQUIRE( it.reverse_complement_hash() == 0x39 ) ;
	REQUIRE( it.finished() ) ;
}


TEST_CASE( "31mer hashes are computed", "[kmer]" ) {
	std::string str( "GAACCCTAAACCCTAAACCCTGACCCCTGAATA" ) ;
	genfile::kmer::KmerHashIterator< std::string::const_iterator > it( str.begin(), str.end(), 31 ) ;

	// G AA CC CT AA AC CC TA AA CC CT GA CC CC TG AA
	REQUIRE( it.hash() == 0x2057015C057855E0 ) ;
	// T TC AG GG GT CA GG GT TT AG GG TT TA GG GT TC
	REQUIRE( it.reverse_complement_hash() == 0x3D2AB4ABF2AFCABD ) ;

	// A AC CC TA AA CC CT AA AC CC TG AC CC CT GA AT
	REQUIRE( (++it).hash() == 0x015C057015E15783 ) ;
	// A TT CA GG GG TC AG GG TT TA GG GT TT AG GG TT
	REQUIRE( it.reverse_complement_hash() == 0x0F4AAD2AFCABF2AF ) ;

	// A CC CT AA AC CC TA AA CC CT GA CC CC TG AA TA
	REQUIRE( (++it).hash() == 0x057015C057855E0C ) ;
	// T AT TC AG GG GT CA GG GT TT AG GG TT TA GG GT
	REQUIRE( it.reverse_complement_hash() == 0x33D2AB4ABF2AFCAB ) ;
}



