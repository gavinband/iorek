
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
		Fasta const& fasta
	) {
		return FastaMask::UniquePtr( new FastaMask( fasta )) ;
	}

	FastaMask::UniquePtr FastaMask::load_from_bed3_file(
		Fasta const& fasta,
		std::string const& filename,
		std::function< void( std::size_t ) > progress_callback
	) {
		FastaMask::UniquePtr result = FastaMask::create( fasta ) ;
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
				result->set_zero_based( elts[0], start, end, genfile::FastaMask::eMasked ) ;
				if( progress_callback ) {
					progress_callback( count ) ;
				}
			}
		}
		return result ;
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
		m_data( (length+63) / 64, 0ul )
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
