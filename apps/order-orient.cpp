
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include "../package_revision_autogenerated.hpp"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/filesystem.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/regex.hpp>

// seqlib
#include "SeqLib/RefGenome.h"
#include "SeqLib/BamReader.h"
#include "SeqLib/GenomicRegionCollection.h"
#include "SeqLib/GenomicRegion.h"
//#include "SeqLib/BWAWrapper.h"

namespace seqlib = SeqLib;
// namespace bt = BamTools ;

#include "appcontext/appcontext.hpp"
#include "appcontext/get_current_time_as_string.hpp"
#include "genfile/GenomePositionRange.hpp"
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/string_utils/slice.hpp"
#include "genfile/Error.hpp"
#include "genfile/Fasta.hpp"
#include "genfile/FastaMask.hpp"
#include "genfile/find_homopolymers_and_short_repeats.hpp"
#include "genfile/repeats/repeat_tracts.hpp"
#include "genfile/reverse_complement.hpp"

#include "statfile/BuiltInTypeStatSink.hpp"

// #define DEBUG 1

namespace globals {
	std::string const program_name = "order-orient" ;
	std::string const program_version = package_version ;
	std::string const program_revision = std::string( package_revision ).substr( 0, 7 ) ;
}

struct IorekOptionProcessor: public appcontext::CmdLineOptionProcessor
{
public:
	std::string get_program_name() const { return globals::program_name ; }

	void declare_options( appcontext::OptionProcessor& options ) {
		// Meta-options
		options.set_help_option( "-help" ) ;
		
		options.declare_group( "Input / output file options" ) ;
		options[ "-o" ]
			.set_description( "Path of output file." )
			.set_takes_single_value()
			.set_default_value( "-" ) ;
		options[ "-fasta" ]
			.set_description( "Specify contigs to order / orient" )
			.set_takes_single_value()
			.set_is_required() ;
		options[ "-alignments" ]
			.set_description( "Specify BAM file of contig-to-reference alignments" )
			.set_takes_single_value()
			.set_is_required() ;
		options[ "-substitute-contig-names" ]
			.set_description(
				"Specify a pair of regular expressions "
				"to use to transform contig names.  May use groups (specified in parentheses) "
				"and back-references (\\1, \\2 etc.) to perform complex substitutions."
			)
			.set_takes_values_until_next_option()
			.set_default_value( "/Pf3D7_([0-9]+)_v3/target_\\1/" )
		;
	}
} ;

struct IorekApplication: public appcontext::ApplicationContext
{
public:
	IorekApplication( int argc, char** argv ):
		appcontext::ApplicationContext(
			globals::program_name,
			globals::program_version + ", revision " + globals::program_revision,
			std::auto_ptr< appcontext::OptionProcessor >( new IorekOptionProcessor ),
			argc,
			argv,
			"-log"
		)
	{}
	
	void run() {
		try {
			unsafe_process() ;
		}
		catch( genfile::InputError const& e ) {
			ui().logger() << "\nError (" << e.what() <<"): " << e.format_message() << ".\n" ;
			throw appcontext::HaltProgramWithReturnCode( -1 ) ;
		}
	}

private:
	genfile::Fasta::UniquePtr m_fasta ;

	struct ContigMap {
		enum Orientation {
			eForward = 0,
			eReverse = 1
		} ;

		ContigMap() {}

		ContigMap(
			std::string name,
			std::string target,
			Orientation orientation,
			int alignment_score,
			int number_of_mismatches,
			int aligned_length
		):
			m_name( name ),
			m_target( target ),
			m_orientation( orientation ),
			m_alignment_score( alignment_score ),
			m_number_of_mismatches( number_of_mismatches ),
			m_aligned_length( aligned_length )
		{}

		std::string const& name() const { return m_name ; }
		std::string const& target() const { return m_target ; }
		Orientation const orientation() const { return m_orientation ; }
		int const alignment_score() const { return m_alignment_score ; }
		int const number_of_mismatches() const { return m_number_of_mismatches ; }
		int const aligned_length() const { return m_aligned_length ; }
	private:
		std::string m_name ;
		std::string m_target ;
		Orientation m_orientation ;
		int m_alignment_score ;
		int m_number_of_mismatches ;
		int m_aligned_length ;
	} ;

	typedef std::map< std::string, ContigMap > Map ;

private:
	void unsafe_process() {
		m_fasta = genfile::Fasta::create() ;
		{
			std::string const& fasta_filename = options().get< std::string >( "-fasta" ) ;
			auto progress_context = ui().get_progress_context( "Loading sequences from \"" + fasta_filename + "\"" ) ;
			m_fasta->add_sequences_from_file( fasta_filename, progress_context ) ;
		}

		seqlib::BamReader reader;
		if( !reader.Open( options().get<std::string>( "-alignments" ) )) {
			assert( "Failed to open file" ) ;
		}
		seqlib::BamHeader const& header = reader.Header() ;

		auto substitutions = get_substitutions( options().get_values< std::string >( "-substitute-contig-names" )) ;

		Map map = process_alignments(
			reader, header,
			[&] (
				std::string const original_contig_name,
				std::string const& reference_name,
				std::size_t count
			) {
				std::string result = reference_name ;
				if( count > 1 ) {
					result = result + "_" + genfile::string_utils::to_string(count) ;
				}
				boost::smatch match ;
				bool matched = false ;
				for( auto& s: substitutions ) {
					if( boost::regex_search( result, match, s.first )) {
//						std::cerr << "++ " << original_contig_name << ", " << reference_name << " matched: " << s.first << ".\n" ;
						result = boost::regex_replace(
							result,
							s.first,
							s.second
						) ;
						matched = true ;
					} else {
//						std::cerr << "-- " << original_contig_name << ", " << reference_name << " did not match: " << s.first << ".\n" ;
					}
				}
				if( !matched ) {
					result = original_contig_name ;
				}
				return result ;
			}
		) ;

		std::auto_ptr< std::ostream > output = genfile::open_text_file_for_output( options().get< std::string >( "-o" )) ;
		using genfile::string_utils::to_string ;
		Map::const_iterator i = map.begin() ;
		for( ; i != map.end(); ++i ) {
			bool const should_reverse_complement = (i->second.orientation() == ContigMap::eReverse) ;
			genfile::Fasta::ContigRange contig = m_fasta->get_sequence( i->first ) ;
			std::string const sequence = (
				should_reverse_complement
					? genfile::reverse_complement( contig.sequence().begin(), contig.sequence().end() )
					: std::string( contig.sequence().begin(), contig.sequence().end() )
			) ;
			(*output)
				<< ">" << i->second.name()
				<< "\tZS:Z:" << i->first
				<< "\tZL:i:" << sequence.size()
				<< "\tZA:i:" << i->second.aligned_length()
				<< "\tZO:Z:" << (should_reverse_complement ? "-" : "+")
				<< "\tZR:A:" << i->second.target()
				<< "\tAS:i:" << i->second.alignment_score()
				<< "\tNM:i:" << i->second.number_of_mismatches()
				<< "\n"
				<< (
					should_reverse_complement
					? genfile::reverse_complement( contig.sequence().begin(), contig.sequence().end() )
					: std::string( contig.sequence().begin(), contig.sequence().end() )
				)
				<< "\n"
			;
		}
	}

	typedef std::vector<
		std::pair<
			boost::regex,
			std::string
		>
	> Substitutions ;

	Substitutions get_substitutions( std::vector<std::string> const& specs ) const {
		using genfile::string_utils::slice ;
		Substitutions result ;
		for( auto spec: specs ) {
			assert( spec.size() > 0 ) ;
			std::string const separator = spec.substr(0,1) ;
			std::vector< slice > elts = slice(spec).split( separator ) ;
			if( (elts.size() != 4) || (elts[0].size() != 0) || (elts[3].size() != 0) ) {
				throw genfile::BadArgumentError(
					"IorekApplication::get_substitutions()",
					"spec=\"" + spec + "\"",
					"Expected specification of the form \"/from/to/\" where from and to are regeular expressions."
				) ;
			}
			result.push_back(
				std::make_pair(
					boost::regex( std::string( elts[1] ) ),
					std::string( elts[2] )
				)
			) ;
		}
		return result ;
	}

	Map process_alignments(
		seqlib::BamReader reader,
		seqlib::BamHeader header,
		std::function< std::string(
			std::string const original_contig_name,
			std::string const& reference_name,
			std::size_t reference_count
		) > generate_name
	) {
		Map Map ;
		std::map< std::string, int > target_counts ;
		seqlib::BamRecord alignment ;
		while( reader.GetNextRecord( alignment ) ) {
			if( 
				!alignment.SecondaryFlag()
				&& !alignment.SupplementaryFlag()
				&& !alignment.DuplicateFlag()
				&& !alignment.QCFailFlag()
				&& alignment.MappedFlag()
			) {
				std::string const original_contig_name = alignment.Qname() ;
				ContigMap::Orientation orientation = alignment.ReverseFlag() ? ContigMap::eReverse : ContigMap::eForward ;
				std::string const reference_name = alignment.ChrName( header ) ;
				int const count = ++target_counts[reference_name] ;
				std::string const new_name = generate_name( original_contig_name, reference_name, count ) ;

				Map::const_iterator where = Map.find( original_contig_name ) ;
				// Check the don't have multiple conflicting alignments for one contig.
				// (We should not as have restricted to primary alignments only.)
				if( where != Map.end() ) {
					throw genfile::BadArgumentError(
						"IorekApplication::process_reads()",
						"alignment for " + original_contig_name,
						"Alignment for this contig has already been seen, expected a single alignment."
					) ;
				} else {
					int alignment_score = -1 ;
					int number_of_mismatches = -1 ;
					alignment.GetIntTag( "AS", alignment_score ) ;
					alignment.GetIntTag( "NM", number_of_mismatches ) ;
					Map.insert(
						std::make_pair(
							original_contig_name,
							ContigMap(
								new_name,
								reference_name,
								orientation,
								alignment_score,
								number_of_mismatches,
								(alignment.PositionEnd() - alignment.Position())
							)
						)
					) ;
				}
			}
		}
		return Map ;
	}
} ;

int main( int argc, char** argv )
{
	std::ios_base::sync_with_stdio( false ) ;
	try {
		IorekApplication app( argc, argv ) ;
		app.run() ;
	}
	catch( appcontext::HaltProgramWithReturnCode const& e ) {
		return e.return_code() ;
	}
	return 0 ;
}
	