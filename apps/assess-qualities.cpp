
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include "../package_revision_autogenerated.hpp"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <boost/filesystem.hpp>

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
#include "statfile/BuiltInTypeStatSink.hpp"

// #define DEBUG 1

namespace globals {
	std::string const program_name = "assess-qualities" ;
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
		options[ "-reads" ]
			.set_description( "Path of bam/cram files to operate on." )
			.set_takes_single_value()
			.set_is_required()
		;
		options[ "-o" ]
			.set_description( "Path of output file." )
			.set_takes_single_value()
			.set_default_value( "-" ) ;

		options[ "-range" ]
			.set_description( "Genomic regions (expressed in the form <chromosome>:<start>-<end>)"
				" to process.  Regions are expressed in 1-based, right-closed coordinates."
				" (These regions should have few copy number variants)" 
				" Alternatively this can be the name of a file containing a list of regions."
			)
			.set_takes_single_value()
		;
		options[ "-include-mask" ]
			.set_description( "Specify a BED file of regions to include in the analysis."
			" If specified, only alignment locations within the given regions are analysed."
			" The file should have no column names and should have contig, start and end columns, "
			" expressed in 0-based right-closed form." )
			.set_takes_single_value()
		;
		options[ "-exclude-mask" ]
			.set_description( "Specify a BED file of regions to mask out of the analysis."
			" If specified, alignment locations within the given regions are omitted from the analysis."
			" The file should have no column names and should have contig, start and end columns, "
			" expressed in 0-based right-closed form." )
			.set_takes_single_value()
		;
		options[ "-fasta" ]
			.set_description( "Specify reference sequence" )
			.set_takes_single_value()
			.set_is_required() ;

		options.declare_group( "Model options" ) ;
		options[ "-mq" ]
			.set_description( "Ignore alignments below this mapping quality threshold" )
			.set_takes_single_value()
			.set_default_value( 20 ) ;
	}
} ;

namespace {
	enum MismatchType {
		eMatch = '=',
		eMismatch = 'X',
		eDeletion = 'D',
		eInsertion = 'I'
	} ;
}

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
	genfile::FastaMask::UniquePtr m_mask ;

private:
	typedef std::map< char, std::vector< int64_t > > Result ;

	void unsafe_process() {
		// Load fasta records
		m_fasta = genfile::Fasta::create() ;
		{
			std::string const& fasta_filename = options().get< std::string >( "-fasta" ) ;
			auto progress_context = ui().get_progress_context( "Loading sequences from \"" + fasta_filename + "\"" ) ;
			m_fasta->add_sequences_from_file( fasta_filename, progress_context ) ;
		}

		m_mask = genfile::FastaMask::create( *m_fasta, genfile::FastaMask::eUnmasked ) ;
		if( options().check( "-include-mask" )) {
			std::string const filename = options().get< std::string >( "-include-mask" ) ;
			auto progress_context = ui().get_progress_context( "Loading mask from \"" + filename + "\"" ) ;
			m_mask->set( genfile::FastaMask::eMasked ) ;
			m_mask->set_from_bed3_file( filename, genfile::FastaMask::eUnmasked, progress_context ) ;
		} 

		if( options().check( "-exclude-mask" )) {
			std::string const filename = options().get< std::string >( "-exclude-mask" ) ;
			auto progress_context = ui().get_progress_context( "Loading mask from \"" + filename + "\"" ) ;
			m_mask->set_from_bed3_file( filename, genfile::FastaMask::eMasked, progress_context ) ;
		} 

		// Open output sink
		statfile::BuiltInTypeStatSink::UniquePtr sink = statfile::BuiltInTypeStatSink::open(
			options().get< std::string >( "-o" )
		) ;

		// Create place to store results 
		// We store the results in two vectors each containin 100 zeros
		// I think of these as bins for bq = 0, bq = 1, etc. 
		// Note: base qualities can generally only go up to 93(ASCII character ~).
		Result mismatches ;
		Result matches ;
		matches['A'] = std::vector< int64_t >( 100, 0 ) ;
		matches['C'] = std::vector< int64_t >( 100, 0 ) ;
		matches['G'] = std::vector< int64_t >( 100, 0 ) ;
		matches['T'] = std::vector< int64_t >( 100, 0 ) ;
		matches['N'] = std::vector< int64_t >( 100, 0 ) ;
		mismatches['A'] = std::vector< int64_t >( 100, 0 ) ;
		mismatches['C'] = std::vector< int64_t >( 100, 0 ) ;
		mismatches['G'] = std::vector< int64_t >( 100, 0 ) ;
		mismatches['T'] = std::vector< int64_t >( 100, 0 ) ;
		mismatches['N'] = std::vector< int64_t >( 100, 0 ) ;

		process_reads(
			options().get_value< std::string >( "-reads" ),
			&matches,
			&mismatches
		) ;

		output_results( matches, mismatches, *sink ) ;
	}

	void process_reads(
		std::string const& filename,
		Result* matches,
		Result* mismatches
	) {
		auto progress_context = ui().get_progress_context( "Processing \"" + filename + "\"" ) ;

		process_reads(
			filename,
			matches,
			mismatches,
			progress_context
		) ;
	}

	void output_results(
		Result const& matches,
		Result const& mismatches,
		statfile::BuiltInTypeStatSink& sink
	) {
		sink | "reference_base" | "base_quality" | "matches" | "mismatches" ;
		std::vector< char > bases{'A', 'C', 'G', 'T'} ;
		for( auto reference_base: bases ) {
			std::vector< int64_t > const& hit = matches.at(reference_base) ;
			std::vector< int64_t > const& miss = mismatches.at(reference_base) ;
			assert( hit.size() == miss.size() ) ;
			for( std::size_t bq = 0; bq < hit.size(); ++bq ) {
				sink
					<< std::string( 1, reference_base )
					<< int64_t(bq)
					<< int64_t(hit[bq])
					<< int64_t(miss[bq])
					<< statfile::end_row() ;
			}
		}
	}

	void process_reads(
		std::string const& filename,
		Result* matches,
		Result* mismatches,
		std::function< void( std::size_t ) > progress_callback
	) {
		seqlib::BamReader reader;
		if( options().check( "-fasta" )) {
			reader.SetCramReference( options().get< std::string >( "-fasta" )) ;
		}
		if( !reader.Open( filename )) {
			assert( "Failed to open file" ) ;
		}
		seqlib::BamHeader const& header = reader.Header() ;
		if( options().check( "-range" )) {
			//std::string const range = options().get< std::string >( "-range" ) ;
			genfile::GenomePositionRange range = genfile::GenomePositionRange::parse( options().get< std::string >( "-range" )) ;
			try {
				// note:  SeqLib is a bit weird on positions.
				// htslib uses 0-based, half-open positions throughout.  See e.g. the hts_parse_reg function which SeqLib uses here under the hood.
				// However, SeqLib changes this back into a 1-based, closed position internally.
				// I found that to correctly capture all reads covering range positions in the range
				// an expanded range is needed.  Therefore I extend both leftwards and rightwards by one.
				// This is ok here because actual precise filtering of positions is done in process_reads() below.
				std::string const range_as_string = (
					std::string(range.chromosome())
					+ ":"
					+ genfile::string_utils::to_string( std::max( int( range.start().position() ) -1, 0 ))
					+ "-"
					+ genfile::string_utils::to_string( range.end().position() + 1 )
				) ;
				seqlib::GenomicRegion R( range_as_string, header) ;
				std::cerr << "Setting range: " << range.toString() << ", " << range_as_string << ": " << R << "\n" ;
				if( !reader.SetRegion( seqlib::GenomicRegion( range_as_string, header ))) {
					throw genfile::BadArgumentError(
						"seqlib::GenomicRegion()",
						"-range",
						"A BAM/CRAM index file (.bai/.crai) must be available to use -range."
					) ;
				}
			} catch( std::invalid_argument const& e ) {
				throw genfile::BadArgumentError(
					"seqlib::GenomicRegion()",
					"region=\"" + range.toString() + "\"",
					"Failed for file \"" + filename + "\""
				) ;
			}
		}
		
		process_reads(
			reader,
			header,
			matches,
			mismatches,
			progress_callback

		) ;
	}
	
	void process_reads(
		seqlib::BamReader reader,
		seqlib::BamHeader header,
		Result* matches,
		Result* mismatches,
		std::function< void( std::size_t ) > progress_callback
	) {
		int32_t const mq_threshold = options().get< int32_t >( "-mq" ) ;

		bool use_range = options().check( "-range" ) ;
		genfile::GenomePositionRange range
			= use_range
			? genfile::GenomePositionRange::parse( options().get< std::string >( "-range" ))
			: genfile::GenomePositionRange(0,0)
		;
		
		seqlib::BamRecord alignment ;
		std::size_t count = 0 ;
		while( reader.GetNextRecord( alignment ) ) {
			if( 
				!alignment.SecondaryFlag()
				&& !alignment.SupplementaryFlag()
				&& !alignment.DuplicateFlag()
				&& !alignment.QCFailFlag()
				&& alignment.MappedFlag()
				&& alignment.MapQuality() >= mq_threshold
			) {
				analyse_alignment_base_qualities(
					alignment,
					header,
					// This is a callback function to get the results from the above function. 
					// It will get called once for each matching/mismatching base in the data.
					[&](
						genfile::Chromosome const chromosome,
						uint32_t const one_based_position,
						char const reference_base,
						MismatchType const& type,
						int base_quality
					) {
						assert( base_quality < 100 ) ;
						// only process the base if it is in the range (if -range is specified)
						// and it is not in the mask.						
						//std::cerr << chromosome << ":" << one_based_position << ":  mask = " << m_mask->at_one_based( chromosome, one_based_position ) << ".\n" ;
						if(
							(m_mask->at_one_based( chromosome, one_based_position ) == genfile::FastaMask::eUnmasked)
							&& (!use_range || range.contains( chromosome, one_based_position ))
						) {
							if( type == eMismatch ) { // 'X'
								Result::iterator where = mismatches->find(std::toupper(reference_base)) ;
								if( where != mismatches->end() ) {
									++((where->second)[base_quality]) ;
								}
							} else if( type == eMatch ) { // '='
								Result::iterator where = matches->find(std::toupper(reference_base)) ;
								if( where != matches->end() ) {
									++((where->second)[base_quality]) ;
								}
							}
						}
					}
				) ; 
			}
			++count ;
			if( progress_callback ) {
				progress_callback( count ) ;
			}
		}
	}

	void analyse_alignment_base_qualities(
		seqlib::BamRecord const& alignment,
		seqlib::BamHeader const& header,
		// This is a 'callback' function to be called
		// on every match or mismatch
		std::function<
			void(
				std::string const& aligned_chromosome,
				uint32_t const aligned_position,
				char const reference_base,
				MismatchType const& type,
				int base_quality
			)
		> callback
	) {
		// Note: this function classifies read-reference mismatches by parsing the
		// CIGAR string and comparing to the reference bases.
		
		// The CIGAR string alone does not contain enough information to reconstruct
		// the mismatching bases.	Thus it is necessary to inspect the reference
		// sequence at the same time.
	
		// Although CIGAR supports 'X' (mismatch) and '=' (identical match), many
		// aligners (such as bwa or minimap2) just output 'M' for mismatches by
		// default.	(Minimap2 does outputs X/= when given the -eqx flag; pbmm2 also
		// seems to do this by default.)

		// Aligners may also output additional information complementing the CIGAR
		// string in the 'MD' tag, and minimap2 can also output a CS tag containing
		// full information about the mismatches.	Use of these would avoid need to
		// parse the reference sequence, but they're not found in every BAM file.
	
		// On balance it therefore seems simplest to work with the CIGAR string and
		// the reference, and to treat 'M', 'X', and '=' as
		// synonymous - which is what we do here.
		
		seqlib::Cigar const& cigar = alignment.GetCigar() ;

		std::string const& read_sequence = alignment.Sequence() ;
		std::string const contig_id = header.IDtoName( alignment.ChrID() ) ;
		if( !m_fasta->contains( contig_id ) ) {
			throw genfile::BadArgumentError(
				"IorekApplication::analyse_alignment_base_qualities()",
				"contig_id=\"" + contig_id + "\"",
				"Contig was not found in reference FASTA file."
			) ;
		}

		genfile::Fasta::ContigRange const contig = m_fasta->get_sequence( contig_id ) ;

		std::string const& base_qualities = alignment.Qualities() ;  // ASCII-encoded base qualities

		// sanity check
		assert( read_sequence.size() == base_qualities.size() ) ;

		//bool const count_matches = options().check( "-count-matches" ) ;
		uint32_t read_position = 0;
		uint32_t aligned_position = alignment.Position() ; // 0-based

#if DEBUG
		std::cerr << "\n++ Inspecting read: " << alignment.Qname() << ", CIGAR = \"" << cigar << "\".\n" ;
#endif

		seqlib::Cigar::const_iterator i = cigar.begin(), end_i = cigar.end() ;
		for( ; i != end_i; ++i ) {
			char const type = i->Type() ;
#if DEBUG
			std::cerr << "  : read position = " << read_position << "; alignment pos = " << aligned_position << "; cigar elt = \"" << *i << "\".\n" ;
#endif
			// The cigar ops are:
			// M    Alignment match (can be a sequence match or mismatch)
			// I    Insertion to the reference
			// D    Deletion from the reference
			// N    Skipped region from the reference
			// S    Soft clip on the read (clipped sequence present in <seq>)
			// H    Hard clip on the read (clipped sequence NOT present in <seq>)
			// P    Padding (silent deletion from the padded reference sequence)
			// =    Sequence match
			// X    Sequence mismatch

			// We treat mismatching aligned bases (M/X/=), deleted bases (D) and
			// inserted bases (I) as mismatches. Soft-clipped, hard-clipped and
			// 'skipped regions are handled to keep track of position but do not count
			// as mismatches.

			// sanity check
#if DEBUG
			std::cerr << std::string( 1, type ) << ":" << std::distance( contig.sequence().begin(), walker.position() ) << " - " << aligned_position << ".\n" ;
#endif
			switch( type ) {
				case 'M':
				case 'X':
				case '=':
					// matching or mismatching bases. Iterate bases and test for mismatch.
					for( int k = 0; k < i->Length(); ++k, ++aligned_position, ++read_position ) {
						char const reference_base = *(contig.sequence().begin() + aligned_position) ;
						char const read_base = read_sequence[read_position] ;
						int const base_quality = (int(base_qualities[read_position]) - 33) ; // convert from ASCII code back to base quality.
						if( std::toupper( reference_base ) == std::toupper( read_base )) {
							// reference and read bases matched!
							// report that now via the callback
							callback(
								contig_id,
								aligned_position+1, // convert back to 1-based
								std::toupper(reference_base),
								eMatch, // '='
								base_quality
							) ;
						} else {
							// reference and read bases matched!
							// report that now via the callback
							callback(
								contig_id,
								aligned_position+1, // convert back to 1-based
								std::toupper(reference_base),
								eMismatch, // 'X'
								base_quality
							) ;
						}
					}
					break ;
				case 'D':
					// TODO - need something to report about deletions.
					aligned_position += i->Length() ;
					// purely deleted bases so no change to read position
					break ;
				case 'I':
					// TODO - need something to report about insertions.
					read_position += i->Length() ;
					// purely inserted bases so no change to aligned position
					break ;
				case 'N':
					// no coverage but need to skip over part of reference.
					read_position += i->Length() ;
					aligned_position += i->Length() ;
					break ;
				case 'S':
					// soft-clipped bases; skip this part of the read.
					read_position += i->Length() ;
					break ;
				case 'H':
					// hard-clipped bases not in the read sequence: nothing to do.
				default:
					// nothing to do for other cases
					break ;
			}
		}
		if( aligned_position != alignment.PositionEnd() ) {
			std::cerr << "!! read alignment: " << alignment.Position() << "-" << alignment.PositionEnd() << ", but parsing ended at " << aligned_position << ".\n" ;
			std::cerr << "!! CIGAR is " << alignment.GetCigar() << ".\n" ;
		}
		assert( aligned_position == alignment.PositionEnd() ) ;
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



