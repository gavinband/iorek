
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include "../package_revision_autogenerated.hpp"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_set.hpp>
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
#include "genfile/GenomePositionRange.hpp"
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/string_utils/slice.hpp"
#include "genfile/Error.hpp"
#include "statfile/BuiltInTypeStatSink.hpp"

#define DEBUG 0

namespace globals {
	std::string const program_name = "coverotron" ;
	std::string const program_version = package_version ;
	std::string const program_revision = std::string( package_revision ).substr( 0, 7 ) ;
}

struct SvelteOptionProcessor: public appcontext::CmdLineOptionProcessor
{
public:
	std::string get_program_name() const { return globals::program_name ; }

	void declare_options( appcontext::OptionProcessor& options ) {
		// Meta-options
		options.set_help_option( "-help" ) ;
		
		options.declare_group( "Input / output file options" ) ;
		options[ "-reads" ]
			.set_description( "Path of bam/cram files to operate on." )
			.set_takes_values_until_next_option()
			.set_is_required()
		;

		options[ "-o" ]
			.set_description( "Path of output file." )
			.set_takes_single_value()
			.set_default_value( "-" ) ;

		options.declare_group( "Model options" ) ;
		
		options[ "-range" ]
			.set_description( "Genommic regions (expressed in the form <chromosome>:<start>-<end>)"
				" to process.  Regions are expressed in 1-based, right-closed coordinates."
				" (These regions should have few copy number variants)" 
				" Alternatively this can be the name of a file containing a list of regions."
			)
			.set_takes_values_until_next_option()
			.set_is_required()
		;
		/*
		options[ "-inference-regions" ]
			.set_description( "Set of regions (expressed in the form <chromosome>:<start>-<end>)"
				" to infer copy number variants in."
				"Regions are expressed in 1-based, right-closed coordinates."
				" Alternatively this can be the name of a file containing a list of regions."
			)
			.set_takes_values_until_next_option()
			.set_is_required()
		;
		*/

		options[ "-reference" ]
			.set_description( "Specify reference sequence (useful when using CRAM files)" )
			.set_takes_single_value() ;

		options[ "-mq" ]
			.set_description( "Mapping quality threshold" )
			.set_takes_single_value()
			.set_default_value( 0 ) ;

		options[ "-bin" ]
			.set_description( "Bin size.  If specified, all regions must be a multiple of this size." )
			.set_takes_single_value()
			.set_default_value( 1 ) ;
		
		options[ "-blacklist" ]
			.set_description( "BED file containing regions to exclude from coverage computation" )
			.set_takes_values_until_next_option()
			.set_minimum_multiplicity(0)
			.set_maximum_multiplicity(100)
		;

		options.declare_group( "Output options" ) ;
		options[ "-no-per-file" ]
			.set_description( "Don't output coverage values per file, only output overall values." ) ;

		options.declare_group( "Miscellaneous options" ) ;
		options[ "-threads" ]
			.set_description( "Use this many extra threads for file reading" )
			.set_takes_single_value()
			.set_default_value( 0 ) ;

	}
} ;

struct Coverage {
	// Class to keep track of total number and mapping quality of reads
	// overlapping a single position
	struct BasePairCoverage {
		BasePairCoverage():
			m_coverage(0),
			m_total_mapping_quality(0)
		{}
		BasePairCoverage( int coverage, int mapping_quality ):
			m_coverage( coverage ),
			m_total_mapping_quality( mapping_quality )
		{}
		BasePairCoverage( BasePairCoverage const& other ):
			m_coverage(other.m_coverage),
			m_total_mapping_quality(other.m_total_mapping_quality)
		{}
		BasePairCoverage& operator=( BasePairCoverage const& other ) {
			m_coverage = other.m_coverage ;
			m_total_mapping_quality = other.m_total_mapping_quality ;
			return *this ;
		}
		BasePairCoverage& operator+=( BasePairCoverage const& other ) {
			m_coverage += other.m_coverage ;
			m_total_mapping_quality += other.m_total_mapping_quality ;
			return *this ;
		}
		
		int coverage() const { return m_coverage ; }
		int total_mapping_quality() const { return m_total_mapping_quality ; }
		
	private:
		int m_coverage ;
		int m_total_mapping_quality ;
	} ;

	typedef std::unordered_map< int, BasePairCoverage > ContigCoverage ;
	typedef std::unordered_map< std::string, ContigCoverage > CoverageMap ;
	typedef genfile::GenomePositionRange Region ;
	typedef std::function< void (int chromosome, int position, int coverage) > ReportCallback ;

private:
	std::string const m_chromosome ;
	int const m_start ;
	int const m_end ;
	std::vector< BasePairCoverage > m_coverage ;
public:
	// Constructor.
	// Assumes an empty range
	Coverage():
		m_chromosome( "" ),
		m_start( 0 ),
		m_end( 0 ),
		m_coverage()
	{}

	// Constructor.
	// Assumes a 0-based half-open region definition
	Coverage( Region const& region ):
		m_chromosome( region.chromosome() ),
		m_start( region.start().position() ),
		m_end( region.end().position() ),
		m_coverage( region.size() )
	{}

	void add_read( seqlib::BamRecord const& read, seqlib::BamHeader const& header ) {
		assert( header.IDtoName( read.ChrID() ) == m_chromosome ) ;
#if 0 
		// coverage not accounting for deletions in alignment
		// (this is simlar to what mpileup outputs).
		for( int position = start; position < end; ++position ) {
			coverage[position] += BasePairCoverage( 1, read.MapQuality() );
		}
#else
		// coverage accounting for deletions in alignment
		// walk Cigar string, count coverage for aligned bases only.
		seqlib::Cigar const& cigar = read.GetCigar() ;
		seqlib::Cigar::const_iterator i = cigar.begin(), end_i = cigar.end() ;
		int position = read.Position() ; // 0-based
		for( ; i != end_i; ++i ) {
			char const type = i->Type() ;

			// The cigar ops are:
			// M    Alignment match (can be a sequence match or mismatch
			// I    Insertion to the reference
			// D    Deletion from the reference
			// N    Skipped region from the reference
			// S    Soft clip on the read (clipped sequence present in <seq>)
			// H    Hard clip on the read (clipped sequence NOT present in <seq>)
			// P    Padding (silent deletion from the padded reference sequence)
			// =    Sequence match
			// X    Sequence mismatch
			//
			// ...so we only need to handle aligned bases and skipped/deleted bases here.
			// (NB. 'skipped' bases probably only turn up in spliced RNA alignments).
			//

			switch( type ) {
				case 'M':
				case 'X':
				case '=':
					for( int k = 0; k < i->Length(); ++k, ++position ) {
						if( position >= m_start && position < m_end ) {
							m_coverage[ position - m_start ] += BasePairCoverage( 1, read.MapQuality() ) ;
						}
					}
					break ;
				case 'D':
				case 'N':
					// no coverage but need to skip over part of reference.
					position += i->Length() ;
					break ;
				default:
					// nothing to do for other cases
					break ;
			}
		}
#endif
	}

	BasePairCoverage report(
		std::string const& chromosome,
		int position
	) const {
		assert( chromosome == m_chromosome ) ;
		assert( position >= m_start && position < m_end ) ;
		return m_coverage[ position - m_start ] ;
	}
} ;

struct SvelteApplication: public appcontext::ApplicationContext
{
public:
	SvelteApplication( int argc, char** argv ):
		appcontext::ApplicationContext(
			globals::program_name,
			globals::program_version + ", revision " + globals::program_revision,
			std::auto_ptr< appcontext::OptionProcessor >( new SvelteOptionProcessor ),
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
	
	typedef std::unordered_map< std::string, Coverage > Coverages ;
	typedef genfile::GenomePositionRange Region ;
	typedef std::vector< genfile::GenomePositionRange > Regions ;
	typedef boost::unordered_set< std::pair< std::string, uint32_t > > Blacklist ;

	void unsafe_process() {
		unsafe_process(
			collect_unique_ids( options().get_values< std::string >( "-reads" ) ),
			get_regions( options().get_values< std::string >( "-range" ) )
		) ;
	}

	std::vector< std::string > collect_unique_ids( std::vector< std::string > const& ids_or_filenames ) {
		std::vector< std::string > result ;
		// collect strings
		for( auto elt: ids_or_filenames ) {
			if( boost::filesystem::exists( elt ) && !((elt.size() > 4 && elt.substr( elt.size() - 4, 4 ) == ".bam") || (elt.size() > 5 && elt.substr( elt.size() - 5, 5 ) == ".cram" ) )) {
				std::ifstream f( elt ) ;
				std::copy(
					std::istream_iterator< std::string >( f ),
					std::istream_iterator< std::string >(),
					std::back_inserter< std::vector< std::string > >( result )
				) ;
			} else {
				result.push_back( elt ) ;
			}
		}
		// sort and uniqueify them...
		std::sort( result.begin(), result.end() ) ;
		std::vector< std::string >::const_iterator newBack = std::unique( result.begin(), result.end() ) ;
		result.resize( newBack - result.begin() ) ;
		return result ;
	}

	Regions get_regions( std::vector< std::string > const& specs ) const {
		Regions result ;
		for( auto spec: specs ) {
			Region region = Region::parse( spec ) ;
			if( !region.has_chromosome() ) {
				throw genfile::BadArgumentError(
					"SvelteApplication::get_regions()",
					"-range=\"" + spec + "\"",
					"Regions must be specified with chromosome/contig identifiers."
				) ;
			}

			// We divide regions into chunks to avoid high memory usage
			uint32_t chunk_size = 2048 ;
			using genfile::string_utils::to_string ;
			if( options().check( "-bin" )) {
				uint32_t const bin_size = options().get< uint32_t >( "-bin" ) ;
				if( region.size() % bin_size != 0 ) {
					throw genfile::BadArgumentError(
						"SvelteApplication::get_regions()",
						"-range=\"" + spec + "\"",
						"Region size (" + to_string( region.size() ) + ") should be a multiple of the bin size (" + to_string(bin_size) + ")"
					) ;
				}
				if( bin_size < chunk_size ) {
					// make chunk size a multiple of bin size
					chunk_size = bin_size * (chunk_size / bin_size) ;
				} else {
					// better not divide bins into chunks
					chunk_size = bin_size ;
				}
			}
			
			if( region.size() > chunk_size ) {
				// Split into sub- regions to avoid using lots of memory
				for( uint32_t position = region.start().position(); position < region.end().position(); position += chunk_size ) {
					result.push_back(
						Region(
							region.chromosome(),
							position,
							std::min( position + chunk_size, region.end().position() )
						)
					) ;
#if DEBUG
					std::cerr << "Added " << result.back() << ".\n" ;
#endif
				}
			} else {
				result.push_back( region ) ;
			}
		}
		return result ;
	}
	
	void unsafe_process(
		std::vector< std::string > const& filenames,
		Regions const& regions
	) {
		std::vector< std::string > const& names = get_names_from_filenames(filenames) ;
		statfile::BuiltInTypeStatSink::UniquePtr sink = create_output_sink( names ) ;

		Blacklist blacklist ;
		if( options().check( "-blacklist" ) ) {
			auto progress_context = ui().get_progress_context( "Loading blacklist" ) ;
			progress_context(0,1) ;
			load_blacklist( options().get_values< std::string >( "-blacklist"), &blacklist ) ;
		}

		using genfile::string_utils::to_string ;	
		for( std::size_t i = 0; i < regions.size(); ++i ) {
			auto region = regions[i] ;
			Coverages coverages ;
			auto progress_context = ui().get_progress_context( "Processing region " + to_string(i+1) + " of " + to_string( regions.size() )) ;
			for( std::size_t file_i = 0; file_i < filenames.size(); ++file_i ) {
				coverages.insert( std::make_pair( names[file_i], Coverage( region ))) ;
				process_reads(
					filenames[file_i],
					region,
					&(coverages[ names[file_i] ] )
				) ;
				progress_context( file_i+1, names.size() ) ;
			}
			output_region( names, coverages, region, *sink, blacklist ) ;
		}
	}
	
	statfile::BuiltInTypeStatSink::UniquePtr create_output_sink(
		std::vector< std::string > const& names
	) {
		bool const output_per_file = !options().check( "-no-per-file" ) ;
		bool const output_total = true ;
		statfile::BuiltInTypeStatSink::UniquePtr sink = statfile::BuiltInTypeStatSink::open( options().get< std::string >( "-o" ) ) ;
		*sink | "chromosome" | "position" | "bin_size" | "L" ;
		if( output_per_file ) {
			for( auto name: names ) {
				(*sink) | (name + ":coverage") | (name + ":mean_mq") ;
			}
		}
		if( output_total ) {
			(*sink) | "N" | "total:coverage" | "total:mean_mq" ;
		}
		return sink ;
	}
	
	void load_blacklist( std::vector< std::string > const& filenames, Blacklist* result ) const {
		for( auto filename: filenames ) {
			std::auto_ptr< std::istream > in = genfile::open_text_file_for_input( filename ) ;
			load_blacklist( *in, result ) ;
		}
	}
	
	void load_blacklist( std::istream& in, Blacklist* result ) const {
		// Read first line and skip UCSC track definition if present
		std::string line ;
		std::getline( in, line ) ;
		if( (line.size() >=7 && line.substr( 0, 7 ) == "browser") || (line.size() >= 5 && line.substr(0,5) == "track" )) {
			// skip this header line
			std::getline( in, line ) ;
		}
		using genfile::string_utils::slice ;
		using genfile::string_utils::to_repr ;
		std::vector< slice > elts ;

		while(in) {
			elts.clear() ;
			slice( line ).split( "\t", &elts ) ;
			assert( elts.size() > 2 ) ;
			genfile::Chromosome chromosome( elts[0] ) ;
			uint32_t start( to_repr< uint32_t >( elts[1] ) ) ;
			uint32_t end( to_repr< uint32_t >( elts[2] ) ) ;
			for( uint32_t pos = start; pos < end; ++pos ) {
				result->insert( std::make_pair( chromosome, pos )) ;
			}
			std::getline( in, line ) ;
		}
	}
	
	void process_reads(
		std::string const& filename,
		Region const& region,
		Coverage* coverage
	) {
		seqlib::BamReader reader;
		std::unique_ptr< seqlib::ThreadPool > thread_pool ;
		
		if( options().get< uint32_t >( "-threads" ) > 0 ) {
			ui().logger() << "!! Error: the -threads option is currently disabled!\n"
				<< "!! This is because the htslib threadpool implementation appears to cause problems with hangs and memory usage.\n" ;
			throw appcontext::HaltProgramWithReturnCode( -1 ) ;
			// thread_pool.reset( new seqlib::ThreadPool( options().get< int >( "-threads" ) )) ;
			// reader.SetThreadPool( *thread_pool ) ;
		}
		
		if( options().check( "-reference" )) {
			reader.SetCramReference( options().get< std::string >( "-reference" )) ;
		}
		if( !reader.Open( filename )) {
			assert( "Failed to open file" ) ;
		}
		
		seqlib::BamHeader const& header = reader.Header() ;
		try {
			// Note: seqlib takes 1-baed coordinates here and holds them 1-based internally.
			// But it converts to 0-based to talk to htslib under the hood.
			// The alignments also come back as 0-based.
			reader.SetRegion( seqlib::GenomicRegion( region.toString(), header )) ;
		} catch( std::invalid_argument const& e ) {
			throw genfile::BadArgumentError(
				"seqlib::GenomicRegion()",
				"region=\"" + region.toString() + "\"",
				"Failed for file \"" + filename + "\""
			) ;
		}

		int32_t const mq_threshold = options().get< int32_t >( "-mq" ) ;
		
		seqlib::BamRecord alignment ;
		seqlib::BamRecordVector records ;
		std::deque< seqlib::BamRecord > alignments ;
		while( reader.GetNextRecord( alignment ) ) {
			if( 
				!alignment.SecondaryFlag()
				&& !alignment.DuplicateFlag()
				&& !alignment.QCFailFlag()
				&& alignment.MappedFlag()
				&& alignment.MapQuality() >= mq_threshold
			) {
				coverage->add_read( alignment, header ) ;
			}
		}
	}
	
	std::vector< std::string > get_names_from_filenames( std::vector< std::string > const& filenames ) {
		std::vector< std::string > result ;
		for( auto s: filenames ) {
			boost::filesystem::path p( s ) ;
			result.push_back( p.stem().string() ) ;
		}
		return result ;
	}
	
	void output_region(
		std::vector< std::string > const& names,
		Coverages const& coverages,
		Region const& region,
		statfile::BuiltInTypeStatSink& sink,
		Blacklist const& blacklist
	) {
		assert( coverages.size() == names.size() ) ;
		genfile::Chromosome chromosome = region.chromosome() ;
		
		uint32_t const bin_size = options().get_value< uint32_t >( "-bin" ) ;

		for(
			uint32_t bin_start = region.start().position();
			bin_start < region.end().position();
			bin_start += bin_size
		) {
			output_bin( names, coverages, chromosome, bin_start, bin_size, sink, blacklist ) ;
		}
	}

	void output_bin(
		std::vector< std::string > const& names,
		Coverages const& coverages,
		genfile::Chromosome const& chromosome,
		uint32_t const bin_start,
		uint32_t const bin_size,
		statfile::BuiltInTypeStatSink& sink,
		Blacklist const& blacklist
	) {
		bool const output_per_file = !options().check( "-no-per-file" ) ;
		bool const output_total = true ;

		sink << chromosome << bin_start << bin_size ;
		uint32_t const bin_end = bin_start + bin_size ;

		uint32_t N = 0 ;
		for( uint32_t position = bin_start; position < bin_end; ++position ) {
			if( blacklist.find( std::make_pair( chromosome, position )) == blacklist.end() ) {
				++N ;
			}
		}
		sink << N ;

		Coverage::BasePairCoverage total ;
		for( auto name: names ) {
			Coverage::BasePairCoverage binned ;
			for( uint32_t position = bin_start; position < bin_end; ++position ) {
				if( blacklist.find( std::make_pair( chromosome, position )) == blacklist.end() ) {
					Coverage::BasePairCoverage const c = coverages.at(name).report( chromosome, position ) ;
					binned += c ;
					total += c ;
				}
			}
			if( output_per_file ) {
				sink << binned.coverage() ;
				if( binned.coverage() > 0 ) {
					sink << double(binned.total_mapping_quality()) / double(binned.coverage()) ;
				} else {
					sink << genfile::MissingValue() ;
				}
			}
		}
		if( output_total ) {
			// output totals
			sink << names.size() ;
			sink << total.coverage() ;
			if( total.coverage() > 0 ) {
				sink << double(total.total_mapping_quality()) / double(total.coverage()) ;
			} else {
				sink << genfile::MissingValue() ;
			}
		}
		sink << statfile::end_row() ;
	}
} ;


int main( int argc, char** argv )
{
	std::ios_base::sync_with_stdio( false ) ;
	try {
		SvelteApplication app( argc, argv ) ;
		app.run() ;
	}
	catch( appcontext::HaltProgramWithReturnCode const& e ) {
		return e.return_code() ;
	}
	return 0 ;
}
	
