
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_FASTA_MASK_HPP
#define GENFILE_FASTA_MASK_HPP

#include <map>
#include <boost/noncopyable.hpp>
#include "genfile/Fasta.hpp"

namespace genfile {
	// Represents a 0-1 mask of sequences from a Fasta file.
	struct FastaMask: public boost::noncopyable {
	public:
		typedef std::unique_ptr< FastaMask > UniquePtr ;
		static UniquePtr create(
			Fasta const& fasta
		) ;

		enum Value:uint64_t { eUnmasked = 0ul, eMasked = 1ul } ;

		FastaMask( Fasta const& fasta ) ;

		void set_one_based( std::string chromosome, uint32_t lower, uint32_t upper, Value value ) ; // closed interval
		void set_zero_based( std::string chromosome, uint32_t lower, uint32_t upper, Value value ) ; // half-open interval
		Value at_one_based( std::string chromosome, uint32_t position ) const ;
		Value at_zero_based( std::string chromosome, uint32_t position ) const ;

	private:
		typedef std::map<
			std::string,
			std::vector< uint64_t >
		> Mask ;
		Mask m_mask ;

	private:
		void initialise( Fasta const& ) ;
	} ;
}
#endif
