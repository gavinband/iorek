
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <map>
#include <vector>
#include <iostream>
#include <boost/noncopyable.hpp>
#include "genfile/Fasta.hpp"
#include "genfile/FastaMask.hpp"
#include "genfile/FileUtils.hpp"
#include "genfile/Error.hpp"
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/string_utils/slice.hpp"

namespace genfile {
	FastaMask::UniquePtr FastaMask::create(
		Fasta const& fasta,
		FastaMask::Value const value
	) {
		return FastaMask::UniquePtr( new FastaMask( fasta, value )) ;
	}

	FastaMask::FastaMask(
		Fasta const& fasta,
		FastaMask::Value const value
	) {
		initialise( fasta, value ) ;
	}

	void FastaMask::initialise(
		Fasta const& fasta,
		Value const value
	) {
		std::vector< std::string > sequence_ids = fasta.sequence_ids() ;
		for( auto id: sequence_ids ) {
			Fasta::ContigRange range = fasta.get_sequence( id ) ;
			Mask::iterator where = m_mask.find( id ) ;
			assert( where == m_mask.end() ) ;
			m_mask[ id ] = ContigMask( range.length(), value ) ;
		}
	}

	void FastaMask::set( Value value ) {
		for( auto& kv: m_mask ) {
			kv.second.set( value ) ;
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

	void FastaMask::set_from_bed3_file(
		std::string const& filename,
		Value const value,
		std::function< void( std::size_t ) > progress_callback
	) {
		std::auto_ptr< std::istream >
			in = genfile::open_text_file_for_input( filename ) ;

		using genfile::string_utils::slice ;
		using genfile::string_utils::to_repr ;
		using genfile::string_utils::to_string ;
		{
			std::string line ;
			std::size_t count = 0 ;
			while( std::getline( *in, line )) {
				++count ;
				std::vector< slice > elts = slice( line ).split( "\t" ) ;
				if( elts.size() != 3 ) {
					throw genfile::BadArgumentError(
						"IorekApplication::load_mask()",
						"filename=\"" + filename + "\"",
						(
							"Wrong number of columns on line "
							+ to_string( count )
							+ " (" + to_string( elts.size() ) + ", expected 3)."
						)
					) ;
				}
				uint32_t start = std::max( to_repr< int32_t >( elts[1] ), int32_t(0) ) ;
				uint32_t end = std::max( to_repr< int32_t >( elts[2] ), int32_t(0) ) ;
				this->set_zero_based( elts[0], start, end, value ) ;
				if( progress_callback ) {
					progress_callback( count ) ;
				}
			}
		}
	}

	FastaMask::Value FastaMask::at_one_based( std::string id, uint32_t position ) const {
		return at_zero_based( id, position-1 ) ;
	}

	FastaMask::Value FastaMask::at_zero_based( std::string id, uint32_t position ) const {
		Mask::const_iterator where = m_mask.find( id ) ;
		assert( where != m_mask.end() ) ;
		return where->second.at_zero_based( position ) ;
	}

	FastaMask::ContigMask::ContigMask(
		uint32_t length,
		FastaMask::Value const value
	):
		m_length( length ),
		m_data( (length+63) / 64, value )
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

	void FastaMask::ContigMask::set( Value value ) {
		uint64_t const v = ((value == 0) ? 0ul : 0xFFFFFFFFFFFFFFFFul) ;
		for( std::size_t i = 0; i < m_data.size(); ++i ) {
			m_data[i] = v ;
		}
	}

	void FastaMask::ContigMask::set_zero_based( uint32_t lower, uint32_t upper, Value value ) {
		assert( lower <= upper ) ;
		lower = std::min( lower, m_length ) ;
		upper = std::min( upper, m_length ) ;
		for( std::size_t pos = lower; pos < upper; ++pos ) {
			std::size_t const word = pos/64 ;
			std::size_t const bit = pos % 64 ;
			uint64_t const bitmask = (1ul << bit) ;
			uint64_t const bitvalue = (value << bit) ;
			m_data[word] &= ~bitmask ;
			m_data[word] |= bitvalue ;
		}
	}

	FastaMask::Value FastaMask::ContigMask::at_zero_based( uint32_t position ) const {
		std::size_t word = position/64 ;
		std::size_t bit = position%64 ;
		uint64_t bitmask = (uint64_t(1) << bit) ;
		return Value( (m_data[word] & bitmask ) != 0 ) ;
	}
}
