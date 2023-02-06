
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <boost/optional.hpp>
#include "genfile/FileUtils.hpp"
#include "genfile/Error.hpp"
#include "genfile/string_utils/slice.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/Fasta.hpp"

#define DEBUG 1

namespace genfile {
	Fasta::UniquePtr Fasta::create() {
		return Fasta::UniquePtr(
			new Fasta()
		) ;
	}

	void Fasta::add_sequences_from_file(
		std::string const& fasta_filename,
		ProgressCallback progress_callback
	)
	{
		load_sequence( std::vector< std::string >( 1, fasta_filename ), progress_callback ) ;
	}

	void Fasta::add_sequences_from_files(
		std::vector< std::string > const& fasta_filenames,
		ProgressCallback callback
	)
	{
		load_sequence( fasta_filenames, callback ) ;
	}

	void Fasta::add_sequence( std::string const& name, std::string const& bases ) {
		ContigSequence sequence( bases.begin(), bases.end() ) ;
		auto range = std::make_pair( 1, 1 + sequence.size() ) ;
		if( m_data.find( name ) != m_data.end() ) {
			throw genfile::DuplicateKeyError( name, "sequence name = \"" + name + "\", stored as = \"" + name + "\"" ) ;
		}
		m_data[ name ] = ChromosomeRangeAndSequence(
			range,
			sequence
		) ;
	}

	std::string const Fasta::get_spec() const {
		return "Fasta()" ;
	}

	void Fasta::load_sequence(
		std::vector< std::string > const& filenames,
		ProgressCallback progress_callback
	) {
		for( std::size_t i = 0; i < filenames.size(); ++i ) {
			std::vector< std::string > elts = genfile::string_utils::split( filenames[i], "=" ) ;
			if( elts.size() == 1 ) {
				elts.insert( elts.begin(), "" ) ;
			} else if( elts.size() > 2 ) {
				throw genfile::BadArgumentError(
					"Fasta::load_sequence()",
					"filenames[" + genfile::string_utils::to_string(i) + "]=\"" + filenames[i] + "\"",
					"Filespec should be in the form: name=<path> or just <path>"
				) ;
			}
			load_sequence( elts[0], elts[1] ) ;
			if( progress_callback ) {
				progress_callback( i + 1, filenames.size() ) ;
			}
		}
	}

	namespace {
		typedef boost::function< void (
			std::string const&,
			uint32_t,
			uint32_t,
			std::deque< char > const&
		) > Callback ;
		void parse_fasta_from_stream( std::istream& stream, Callback callback ) {
			std::size_t const BUFFER_SIZE = 1024*1024 ;
			std::vector< char > buffer( BUFFER_SIZE ) ;
			std::string sequenceName = "" ;
			std::deque< char > sequence ;

			// State machine
			// We are either reading a header line or reading sequence lines.
			enum State { eHeader = 0, eSequence = 1 } ;
			State state = eHeader ;

			// try to read more data
			stream.read( &buffer[0], BUFFER_SIZE ) ;
			std::size_t const count = stream.gcount() ;
			char const* p = &buffer[0] ;
			char const* data_end = &buffer[0] + count ;

			while( stream || p < data_end ) { // while there is more data
				char const* const line_or_data_end = std::find( p, data_end, '\n' ) ;

				if( state == eHeader ) {
					assert( p != data_end ) ;
					if( *p != '>' ) {
						throw genfile::MalformedInputError(
							"(stream)",
							"File does not appear to be a FASTA file (sequence does not start with a \">\" character)",
							0
						) ;
					}
					// assume header line fits in one \n-terminated line
					if( line_or_data_end == data_end ) {
						throw genfile::MalformedInputError(
							"(stream)",
							"File does not appear to be a FASTA file (sequence header line appears excessively long)",
							0
						) ;
					}
					char const* nameEnd = std::find( p+1, line_or_data_end, ' ' ) ;
					sequenceName.assign( p+1, nameEnd ) ;
					sequence.clear() ;
					state = eSequence ;
				} else {
					// state == eSequence
					sequence.insert( sequence.end(), p, line_or_data_end ) ;
				}
				p = std::min( line_or_data_end + 1, data_end ) ;
	
				// Deal with loading additional data if needed
				if( p == data_end && stream ) {
					// try to read more data
					stream.read( &buffer[0], BUFFER_SIZE ) ;
					std::size_t const count = stream.gcount() ;
					p = &buffer[0] ;
					data_end = &buffer[0] + count ;
				}
	
				// Store the existing sequence if it has ended
				if( p == data_end || *p == '>' ) {
					// ...and store sequence
					callback( sequenceName, 1, 1 + sequence.size(), sequence ) ;
					state = eHeader ;
				}
			}
		}
	}

	void Fasta::load_sequence(
		std::string const& name,
		std::string const& filename
	) {
		using genfile::string_utils::to_string ;

		// read header
		std::auto_ptr< std::istream > stream( genfile::open_text_file_for_input( filename )) ;
		if( !*stream ) {
			throw genfile::MalformedInputError( filename, 0 ) ;
		}

		std::string const prefix = ( name == "" ? "" : ( name + ":" )) ;
	
		try {
			parse_fasta_from_stream(
				*stream,
				[&]( std::string const& sequenceName, Position start, Position end, ContigSequence const& sequence ) {
					std::string storedName = prefix + sequenceName ;
					if( m_data.find( storedName ) != m_data.end() ) {
						throw genfile::DuplicateKeyError( filename, "sequence name = \"" + sequenceName + "\", stored as = \"" + storedName + "\"" ) ;
					}
					m_data[ storedName ] = ChromosomeRangeAndSequence(
						std::make_pair( start, end ),
						sequence
					) ;
				}
			) ;
		} catch( genfile::MalformedInputError const& e ) {
			throw genfile::MalformedInputError(
				filename,
				e.message(),
				e.line(),
				e.column()
			) ;
		}
	}

	std::string Fasta::get_summary( std::string const& prefix, std::size_t column_width ) const {
		using genfile::string_utils::to_string ;
		std::string result = prefix + "Fasta sequence for the following regions:" ;
		std::size_t count = 0 ;
		for( SequenceData::const_iterator i = m_data.begin(); i != m_data.end(); ++i, ++count ) {
			result += "\n" + prefix + " - sequence \"" + to_string( i->first ) + "\":"
				+ to_string( i->second.first.first ) + "-" + to_string( i->second.first.second )
				+  " (length " + to_string( i->second.first.second - i->second.first.first ) + ")" ;
		}
		return result ;
	}

	std::vector< genfile::GenomePositionRange > Fasta::get_ranges() const {
		std::vector< genfile::GenomePositionRange > result ;
		SequenceData::const_iterator i = m_data.begin(), end_i = m_data.end() ;
		for( ; i != end_i; ++i ) {
			result.push_back(
				genfile::GenomePositionRange(
					i->first,
					i->second.first.first,
					i->second.first.second - 1
				)
			) ;
		}
		return result ;
	}

	bool Fasta::contains( genfile::Chromosome const& chromosome ) const {
		SequenceData::const_iterator where = m_data.find( chromosome ) ;
		return where != m_data.end() ;
	}

	char Fasta::get_base( genfile::GenomePosition const& position ) const {
		using namespace genfile::string_utils ;
		SequenceData::const_iterator where = m_data.find( position.chromosome() ) ;
		if( where == m_data.end() ) {
			throw genfile::BadArgumentError(
				"Fasta::get_base()",
				"chromosome=\"" + to_string( position.chromosome() ) + "\"",
				"Chromosome is not in the stored sequence."
			) ;
		}
		genfile::Position const sequence_start = where->second.first.first ;
		genfile::Position const sequence_end = where->second.first.second ; // one-past-the-end
		if( position.position() >= sequence_end || position.position() < sequence_start ) {
			throw genfile::BadArgumentError(
				"Fasta::get_base()",
				"position=" + to_string( position.position() ),
				"Position is not in the sequence (" + to_string( position.chromosome() ) + ":" + to_string( sequence_start ) + "-" + to_string( sequence_end ) + ")."
			) ;
		}
		return *(where->second.second.begin() + position.position() - sequence_start) ;
	}


	genfile::GenomePositionRange Fasta::get_range(
		genfile::Chromosome const& name
	) const {
		return get_sequence( name ).positions() ;
	}

	void Fasta::get_sequence(
		genfile::Chromosome const& chromosome,
		std::deque<char>* result
	) const {
		SequenceData::const_iterator where = m_data.find( chromosome ) ;
		assert( where != m_data.end() ) ;
		result->assign( where->second.second.begin(), where->second.second.end() ) ;
	}

	void Fasta::get_sequence(
		genfile::Chromosome const& chromosome,
		genfile::Position start,
		genfile::Position end,
		std::deque<char>* result
	) const {
		ContigRange range = get_sequence( chromosome, start, end ) ;
		result->resize( end - start, '.' ) ;

		std::copy(
			range.sequence().begin(), range.sequence().end(),
			result->begin() + ( range.positions().start().position() - start )
		) ;
	}

	Fasta::ContigRange
	Fasta::get_sequence( genfile::Chromosome const& chromosome ) const {
		SequenceData::const_iterator where = m_data.find( chromosome ) ;
		assert( where != m_data.end() ) ;
		return get_sequence(
			chromosome,
			where->second.first.first, where->second.first.second
		) ;
	}

	Fasta::ContigRange
	Fasta::get_sequence( genfile::Chromosome const& chromosome, genfile::Position start, genfile::Position end ) const {
		using namespace genfile::string_utils ;
		assert( end >= start ) ;
		SequenceData::const_iterator where = m_data.find( chromosome ) ;
		if( where == m_data.end() ) {
			throw genfile::BadArgumentError(
				"Fasta::get_sequence()",
				"chromosome=\"" + to_string( chromosome ) + "\"",
				"Chromosome is not in the stored sequence."
			) ;
		}
		
		genfile::Position const one_based_sequence_start = where->second.first.first ;
		genfile::Position const one_based_sequence_end = where->second.first.second ; // one-past-the-end

			/*if( start > one_based_sequence_end || start < one_based_sequence_start || end < one_based_sequence_start || end > one_based_sequence_end ) {
			throw genfile::BadArgumentError(
				"Fasta::get_sequence()",
				"start..end=" + to_string( start ) + ".." + to_string( end ),
				"Region does not overlap the sequence (" + to_string( chromosome ) + ":" + to_string( one_based_sequence_start ) + "-" + to_string( one_based_sequence_end ) + ")."
			) ;
		}
		*/
		genfile::Position actual_start = std::max( start, one_based_sequence_start ) ;
		genfile::Position actual_end = std::min( end, one_based_sequence_end ) ;

		return ContigRange(
			genfile::GenomePositionRange(
				chromosome, actual_start, actual_end
			),
			ConstSequenceRange(
				where->second.second.begin() + (start - one_based_sequence_start ),
				where->second.second.begin() + (end - one_based_sequence_start )
			)
		) ;
	}

	std::vector< std::string > Fasta::sequence_ids() const {
		std::vector< std::string > result ;
		SequenceData::const_iterator i = m_data.begin(), end_i = m_data.end() ;
		for( ; i != end_i; ++i ) {
			result.push_back( i->first ) ;
		}
		return result ;
	}

	void Fasta::sequence_ids( boost::function< void( std::string ) > callback ) const {
		SequenceData::const_iterator i = m_data.begin(), end_i = m_data.end() ;
		for( ; i != end_i; ++i ) {
			callback( i->first ) ;
		}
	}

}
