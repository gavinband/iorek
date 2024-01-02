
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
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include "genfile/Chromosome.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/wildcard.hpp"
#include "genfile/GenomePositionRange.hpp"

namespace genfile {
	// Represents sequences from a Fasta file.
	// This uses one-based, closed coordinates through the API.
	struct Fasta: public boost::noncopyable {
	public:
		typedef std::unique_ptr< Fasta > UniquePtr ;
		typedef std::deque< char > ContigSequence ;
		typedef genfile::Chromosome Chromosome ;
		typedef ContigSequence::const_iterator ConstSequenceIterator ;

		struct ConstSequenceRange
		{
			ConstSequenceRange( ConstSequenceIterator const& begin, ConstSequenceIterator const& end ):
				m_begin( begin ),
				m_end( end )
			{}

			ConstSequenceRange( ConstSequenceRange const& other ):
				m_begin( other.m_begin ),
				m_end( other.m_end )
			{}

			ConstSequenceRange& operator=( ConstSequenceRange const& other ) {
				m_begin = other.m_begin ;
				m_end = other.m_end ;
				return *this ;
			}
			
			ConstSequenceIterator begin() const { return m_begin ; }
			ConstSequenceIterator end() const { return m_end ; }
			std::size_t size() const { return std::distance( m_begin, m_end ) ; }
			char const operator[]( std::size_t where ) const { return *(m_begin+where) ; }
			
		private:
			ConstSequenceIterator m_begin ;
			ConstSequenceIterator m_end ;
		} ;

		// Represents a subrange of a contig.
		// It contains both the position range (half-open) and
		// a range of iterators into the sequence itself (half-open).
		struct ContigRange
		{
			ContigRange( ContigRange const& other ):
				m_range( other.m_range ),
				m_length( other.m_length ),
				m_sequence( other.m_sequence )
			{}

			ContigRange(
				genfile::GenomePositionRange const& range,
				ConstSequenceRange const& sequence
			):
				m_range( range ),
				m_length( range.end().position() - range.start().position() ),
				m_sequence( sequence )
			{}

			genfile::GenomePositionRange positions() const { return m_range ; }
			std::size_t length() const { return m_length ; }
			std::size_t size() const { return m_length ; }
			ConstSequenceRange sequence() const { return m_sequence ; }
			char operator[]( std::size_t where ) const { return *(m_sequence.begin() + where) ; }

		private:
			genfile::GenomePositionRange const m_range ;
			std::size_t const m_length ;
			ConstSequenceRange const m_sequence ;
		private:
			ContigRange() ;
			ContigRange& operator=( ContigRange const& other ) ;
		} ;
		
		typedef std::pair< std::pair< genfile::Position, genfile::Position >, ContigSequence > ChromosomeRangeAndSequence ;
		typedef std::map< Chromosome, ChromosomeRangeAndSequence > SequenceData ;
		typedef boost::function< void ( std::size_t, boost::optional< std::size_t > ) > ProgressCallback ;
	public:
		static UniquePtr create() ;

	public:
		Fasta() {}

		void add_sequences_from_file( std::string const& fasta_filename, ProgressCallback ) ;
		void add_sequences_from_files( std::vector< std::string > const& fasta_filenames, ProgressCallback ) ;
		void add_sequence( std::string const& name, std::string const& sequence ) ;

		std::size_t number_of_sequences() const { return m_data.size() ; }
		std::vector< std::string > sequence_ids() const ;
		void sequence_ids( boost::function< void( std::string ) > callback ) const ;

		SequenceData const& sequence() const { return m_data ; }
		std::string const get_spec() const ;
		std::string get_summary( std::string const& prefix, std::size_t column_width = 80 ) const ;
		std::vector< genfile::GenomePositionRange > get_ranges() const ;

		bool contains( genfile::Chromosome const& chromosome ) const ;
		char get_base( genfile::GenomePosition const& position ) const ;
		void get_sequence(
			genfile::Chromosome const& name,
			std::deque< char >* result
		) const ;
		void get_sequence(
			genfile::Chromosome const& name,
			genfile::Position start,
			genfile::Position end,
			std::deque< char >* result
		) const ;
		ContigRange get_sequence(
			genfile::Chromosome const& name
		) const ;
		genfile::GenomePositionRange get_range(
			genfile::Chromosome const& name
		) const ;
		ContigRange get_sequence(
			genfile::Chromosome const& name,
			genfile::Position start,
			genfile::Position end
		) const ;

	private:
		SequenceData m_data ;
	
		void load_sequence( std::vector< std::string > const& filenames, ProgressCallback callback ) ;
		void load_sequence( std::string const& name, std::string const& filename ) ;
	} ;
}
#endif
