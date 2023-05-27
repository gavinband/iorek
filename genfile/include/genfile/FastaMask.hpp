
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_FASTA_MASK_HPP
#define GENFILE_FASTA_MASK_HPP

#include <map>
#include <functional>
#include <boost/noncopyable.hpp>
#include "genfile/Fasta.hpp"

namespace genfile {
	// Represents a 0-1 mask of sequences from a Fasta file.
	struct FastaMask: public boost::noncopyable {
	public:
		enum Value:uint64_t { eUnmasked = 0ul, eMasked = 1ul } ;
		typedef std::unique_ptr< FastaMask > UniquePtr ;
		static UniquePtr create(
			Fasta const& fasta,
			Value const value = eUnmasked
		) ;

		FastaMask( Fasta const& fasta, Value const value = eUnmasked ) ;

		void set( Value value ) ;
		void set_one_based( std::string chromosome, uint32_t lower, uint32_t upper, Value value ) ; // closed interval
		void set_zero_based( std::string chromosome, uint32_t lower, uint32_t upper, Value value ) ; // half-open interval
		void set_from_bed3_file(
			std::string const& filename,
			Value const value,
			std::function< void( std::size_t ) > progress_callback = std::function< void( std::size_t ) >()
		) ;

		Value at_one_based( std::string chromosome, uint32_t position ) const ;
		Value at_zero_based( std::string chromosome, uint32_t position ) const ;

	private:

		struct ContigMask {
			ContigMask( uint32_t length = 0, Value const value = eUnmasked ) ;
			ContigMask( ContigMask const& other ) ;
			ContigMask& operator=( ContigMask const& other ) ;

			void set( Value value ) ;
			void set_zero_based( uint32_t lower, uint32_t upper, Value value ) ; // half-open interval
			Value at_zero_based( uint32_t position ) const ;

		private:
			uint32_t m_length ;
			std::vector< uint64_t > m_data ;
		} ;

		typedef std::map<
			std::string,
			ContigMask
		> Mask ;
		Mask m_mask ;

	private:
		void initialise( Fasta const&, Value const ) ;
	} ;
}
#endif
