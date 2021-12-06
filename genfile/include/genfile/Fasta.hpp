
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_FASTA_HPP
#define GENFILE_FASTA_HPP

#include <utility>
#include <map>
#include <deque>
#include <string>
#include <memory>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include "genfile/Chromosome.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/wildcard.hpp"
#include "genfile/GenomePositionRange.hpp"

namespace genfile {
	struct Fasta {
	public:
		typedef std::unique_ptr< Fasta > UniquePtr ;
		typedef std::deque< char > ContigSequence ;
		typedef genfile::Chromosome Chromosome ;
		typedef ContigSequence::const_iterator ConstSequenceIterator ;
		typedef std::pair< ConstSequenceIterator, ConstSequenceIterator > ConstSequenceRange ;
		typedef std::pair< genfile::GenomePositionRange, ConstSequenceRange > PositionedSequenceRange ;
		typedef std::pair< std::pair< genfile::Position, genfile::Position >, ContigSequence > ChromosomeRangeAndSequence ;
		typedef std::map< Chromosome, ChromosomeRangeAndSequence > SequenceData ;
		typedef boost::function< void ( std::size_t, boost::optional< std::size_t > ) > ProgressCallback ;
	public:
		static UniquePtr create() ;

	public:

		Fasta() {}

		void add_sequences_from_file( std::string const& fasta_filename, ProgressCallback ) ;
		void add_sequences_from_files( std::vector< std::string > const& fasta_filenames, ProgressCallback ) ;

		std::size_t number_of_sequences() const { return m_data.size() ; }
		std::vector< std::string > contig_ids() const ;

		SequenceData const& sequence() const { return m_data ; }
		std::string const get_spec() const ;
		std::string get_summary( std::string const& prefix, std::size_t column_width = 80 ) const ;
		std::vector< genfile::GenomePositionRange > get_ranges() const ;

		bool contains( genfile::Chromosome const& chromosome ) const ;
		char get_base( genfile::GenomePosition const& position ) const ;
		void get_sequence( genfile::Chromosome const& name, genfile::Position start, genfile::Position end, std::deque< char >* result ) const ;
		PositionedSequenceRange get_sequence( genfile::Chromosome const& name ) const ;
		PositionedSequenceRange get_sequence( genfile::Chromosome const& name, genfile::Position start, genfile::Position end ) const ;

	private:
		SequenceData m_data ;
	
		void load_sequence( std::vector< std::string > const& filenames, ProgressCallback callback ) ;
		void load_sequence( std::string const& name, std::string const& filename ) ;
	} ;
}
#endif
