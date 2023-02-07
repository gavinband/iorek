
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <map>
#include <boost/noncopyable.hpp>
#include "genfile/Fasta.hpp"
#include "genfile/FastaMask.hpp"

namespace genfile {
	FastaMask::UniquePtr FastaMask::create(
		Fasta const& fasta
	) {
		return FastaMask::UniquePtr( new FastaMask( fasta )) ;
	}

	FastaMask::FastaMask( Fasta const& fasta ) {
		initialise( fasta ) ;
	}

	void FastaMask::initialise( Fasta const& fasta ) {
		std::vector< std::string > sequence_ids = fasta.sequence_ids() ;
		for( auto id: sequence_ids ) {
			Fasta::ContigRange range = fasta.get_sequence( id ) ;
			Mask::iterator where = m_mask.find( id ) ;
			assert( where == m_mask.end() ) ;
			m_mask[ id ] = ContigMask( range.length() ) ;
		}
	}

	void FastaMask::set_one_based( std::string id, uint32_t lower, uint32_t upper, Value value ) {
		set_zero_based( id, lower-1, upper, value ) ;
	}

	void FastaMask::set_zero_based( std::string id, uint32_t lower, uint32_t upper, Value value ) {
		Mask::iterator where = m_mask.find( id ) ;
		assert( where != m_mask.end() ) ;
		where->second.set_zero_based( lower, upper, value ) ;
	} ;

	FastaMask::Value FastaMask::at_one_based( std::string id, uint32_t position ) const {
		return at_zero_based( id, position-1 ) ;
	}

	FastaMask::Value FastaMask::at_zero_based( std::string id, uint32_t position ) const {
		Mask::const_iterator where = m_mask.find( id ) ;
		assert( where != m_mask.end() ) ;
		return where->second.at_zero_based( position ) ;
	}

	FastaMask::ContigMask::ContigMask( uint32_t length ):
		m_length( length ),
		m_data( length, 0ul )
	{}

	FastaMask::ContigMask::ContigMask( FastaMask::ContigMask const& other ):
		m_length( other.m_length ),
		m_data( other.m_data )
	{}

	FastaMask::ContigMask& FastaMask::ContigMask::operator=( FastaMask::ContigMask const& other ) {
		m_length = other.m_length ;
		m_data = other.m_data ;
		return *this ;
	}

	void FastaMask::ContigMask::set_zero_based( uint32_t lower, uint32_t upper, Value value ) {
		assert( lower <= upper ) ;
		lower = std::min( lower, m_length ) ;
		upper = std::min( upper, m_length ) ;
		for( std::size_t pos = lower; pos < upper; ++pos ) {
			std::size_t const word = pos/64 ;
			std::size_t const bit = pos % 64 ;
			uint64_t const bitmask = (value << bit) ;
			m_data[word] |= bitmask ;
		}
	}

	FastaMask::Value FastaMask::ContigMask::at_zero_based( uint32_t position ) const {
		std::size_t word = position/64 ;
		std::size_t bit = position%64 ;
		uint64_t bitmask = (uint64_t(1) << bit) ;
		return Value( (m_data[word] & bitmask ) != 0 ) ;
	}
}
