
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <fstream>
#include "test_case.hpp"
#include "genfile/SNPDataSource.hpp"
#include "genfile/SNPDataSourceChain.hpp"
#include "genfile/GenFileSNPDataSource.hpp"
#include "genfile/BGenFileSNPDataSource.hpp"
#include "genfile/SNPDataSink.hpp"
#include "genfile/FileUtils.hpp"
#include "genfile/GenFileSNPDataSink.hpp"
#include "genfile/BGenFileSNPDataSink.hpp"
#include "stdint.h"

AUTO_TEST_SUITE( test_snp_data_source_chain )

// The following section contains a simple snp block writer.
namespace data {
	namespace {
		// this data has 1504 samples per row.
		unsigned int const number_of_samples = 7 ;
		unsigned int const number_of_snps = 16 ;
		std::string data =
			"--- rs11089130 14431347 C G 0.11 0.44 0.44 0.11 0.44 0.44 0.11 0.44 0.44 0.11 0.44 0.44 0.11 0.44 0.44 0.11 0.44 0.44 0.11 0.44 0.44\n"
			"--- rs738829 14432618 A G 0.04 0.31 0.65 0.04 0.31 0.65 0.04 0.31 0.65 0.04 0.31 0.65 0.04 0.31 0.65 0.04 0.31 0.65 0.04 0.31 0.65\n"
			"--- rs915674 14433624 A G 0.02 0.23 0.75 0.02 0.23 0.75 0.02 0.23 0.75 0.02 0.23 0.75 0.02 0.23 0.75 0.02 0.23 0.75 0.02 0.23 0.75\n"
			"--- rs915675 14433659 A C 0.04 0.33 0.63 0.04 0.33 0.63 0.04 0.33 0.63 0.04 0.33 0.63 0.04 0.33 0.63 0.04 0.33 0.63 0.04 0.33 0.63\n"
			"--- rs915677 14433758 A G 0 0.13 0.87 0 0.13 0.87 0 0.13 0.87 0 0.13 0.87 0 0.13 0.87 0 0.13 0.87 0 0.13 0.87\n"
			"--- rs9604721 14434713 C T 0.93 0.07 0 0.93 0.07 0 0.93 0.07 0 0.93 0.07 0 0.93 0.07 0 0.93 0.07 0 0.93 0.07 0\n"
			"--- rs4389403 14435070 A G 0.01 0.19 0.79 0.01 0.19 0.79 0.01 0.19 0.79 0.01 0.19 0.79 0.01 0.19 0.79 0.01 0.19 0.79 0.01 0.19 0.79\n"
			"--- rs5746356 14439734 C T 0.55 0.38 0.07 0.55 0.38 0.07 0.55 0.38 0.07 0.55 0.38 0.07 0.55 0.38 0.07 0.55 0.38 0.07 0.55 0.38 0.07\n"
			"--- rs9617528 14441016 C T 0.07 0.38 0.55 0.07 0.38 0.55 0.07 0.38 0.55 0.07 0.38 0.55 0.07 0.38 0.55 0.07 0.38 0.55 0.07 0.38 0.55\n"
			"--- rs2154787 14449374 C T 0.47 0.43 0.10 0.47 0.43 0.10 0.47 0.43 0.10 0.47 0.43 0.10 0.47 0.43 0.10 0.47 0.43 0.10 0.47 0.43 0.10\n"
			"--- rs12484041 14452292 A G 0 0.03 0.97 0 0.03 0.97 0 0.03 0.97 0 0.03 0.97 0 0.03 0.97 0 0.03 0.97 0 0.03 0.97\n"
			"--- rs10154731 14479437 A G 0 0.15 0.84 0 0.15 0.84 0 0.15 0.84 0 0.15 0.84 0 0.15 0.84 0 0.15 0.84 0 0.15 0.84\n"
			"--- rs11913813 14480059 C G 0.92 0.08 0 0.92 0.08 0 0.92 0.08 0 0.92 0.08 0 0.92 0.08 0 0.92 0.08 0 0.92 0.08 0\n"
			"--- rs2260460 14482325 C T 0.90 0.10 0 0.90 0.10 0 0.90 0.10 0 0.90 0.10 0 0.90 0.10 0 0.90 0.10 0 0.90 0.10 0\n"
			"--- rs1964966 14483902 A G 0.74 0.24 0.02 0.74 0.24 0.02 0.74 0.24 0.02 0.74 0.24 0.02 0.74 0.24 0.02 0.74 0.24 0.02 0.74 0.24 0.02\n"
			"SNP_A-1928576 rs11705026 14490036 G T 0 1 0 0.01 0.99 0 0 1 0 0 0 1 0.06 0.94 0 0 1 0 0 0 1\n" ;
	}
}

namespace {
	// The following section defines the needed objects for use with the bgen.hpp implementation.
	template< typename T >
	struct Setter
	{
		Setter( T& field ): m_field( field ) {} ;
		template< typename T2 >
		void operator()( T2 const& value ) { m_field = T( value ) ; }
	private:
		T& m_field ;
	} ;

	template< typename T >
	Setter< T > make_setter( T& field ) { return Setter<T>( field ) ; }


	struct probabilities_t {
		double AA, AB, BB ;
		bool operator==( probabilities_t const& other ) const {
			return AA == other.AA && AB == other.AB && BB == other.BB ;
		}
	} ;

	struct ProbabilitySetter {
		ProbabilitySetter( std::vector< probabilities_t >& probabilities ): m_probabilities( probabilities ) {}
		void operator() ( std::size_t i, double aa, double ab, double bb ) {
			m_probabilities[i].AA = aa ;  
			m_probabilities[i].AB = ab ;  
			m_probabilities[i].BB = bb ;  
		}

	private:
		std::vector<probabilities_t>& m_probabilities ;
	} ;

	struct ProbabilityGetter {
		ProbabilityGetter( std::vector< probabilities_t >& probabilities, int index ): m_probabilities( probabilities ), m_index( index ) {}
		double operator() ( std::size_t i ) {
			switch( m_index ) {
				case 0: return m_probabilities[i].AA ; break ;
				case 1: return m_probabilities[i].AB ; break ;
				case 2: return m_probabilities[i].BB ; break ;
				default:
				assert(0); 
				break ;
			}
			return 0.0 ;
		}
	private:
		std::vector<probabilities_t>& m_probabilities ;
		int m_index ;
	} ;

	struct SnpData {
	
		SnpData(): probabilities( data::number_of_samples ) {} ;
		genfile::VariantIdentifyingData snp ;
		std::vector< probabilities_t > probabilities ;
		
		bool operator==( SnpData const& other ) const {
			return snp == other.snp && probabilities == other.probabilities ;
		}
	} ;

	namespace {
		std::string get_test_sample_name( std::size_t i ) {
			return "test_sample_" + genfile::string_utils::to_string( i ) ;
		}
	}

	void copy_gen_file( genfile::SNPDataSource& snp_data_source, genfile::SNPDataSink& snp_data_sink ) {
		SnpData snp_data ;
		
		snp_data_sink.set_sample_names( snp_data_source.number_of_samples(), &get_test_sample_name ) ;

		while( snp_data_source.get_snp_identifying_data( &snp_data.snp ) ) {
			snp_data_source.read_snp_probability_data( ProbabilitySetter( snp_data.probabilities ) ) ;
			snp_data_sink.write_snp(
				snp_data_source.number_of_samples(),
				snp_data.snp,
				ProbabilityGetter( snp_data.probabilities, 0 ),
				ProbabilityGetter( snp_data.probabilities, 1 ),
				ProbabilityGetter( snp_data.probabilities, 2 )
			) ;
		}
	}

	void copy_gen_file( std::string original, genfile::SNPDataSink& target ) {
		genfile::GenFileSNPDataSource gen_file_snp_data_source( original, genfile::Chromosome() ) ;
		copy_gen_file( gen_file_snp_data_source, target ) ;
	}

	void copy_gen_file( std::string const& original, std::string const& target ) {
		genfile::SNPDataSink::UniquePtr snp_data_sink( genfile::SNPDataSink::create( target )) ;
		copy_gen_file( original, *snp_data_sink ) ;
	}

	// The following section contains the main tests.
	void create_files( std::string original, std::string const& gen, std::string const& gen2 ) {
		// set up our original file.
		{
			std::cerr << "Creating original...\n" ;
			std::ofstream original_file( original.c_str() ) ;
			original_file << data::data ;
		}
		std::cerr << "Creating gen file 2...\n" ;
		copy_gen_file( original, gen ) ;
		std::cerr << "Creating gen file 2...\n" ;
		copy_gen_file( original, gen2 ) ;
	}

	std::vector< SnpData > read_snp_data( genfile::SNPDataSource& snp_data_source ) {
		std::vector< SnpData > result ;
		SnpData snp_data ;
	
		while( snp_data_source.get_snp_identifying_data( &snp_data.snp )) {
			snp_data_source.read_snp_probability_data( ProbabilitySetter( snp_data.probabilities )) ;
			result.push_back( snp_data ) ;
		}
		return result ;
	}

	std::vector< SnpData > read_gen_files( std::vector< genfile::wildcard::FilenameMatch > const& filenames ) {
		genfile::SNPDataSource::UniquePtr snp_data_source(
			genfile::SNPDataSourceChain::create( filenames ).release()
		) ;
		return read_snp_data( *snp_data_source ) ;
	}
}

AUTO_TEST_CASE( test_snp_data_source_chain ) {
	std::string original = genfile::create_temporary_filename() + std::string( ".gen" );
	std::string gen = genfile::create_temporary_filename() + std::string( ".gen" );
	std::string gen2 = genfile::create_temporary_filename() + std::string( ".gen" );

	std::vector< genfile::wildcard::FilenameMatch > filenames ;
	filenames.push_back( gen ) ;
	filenames.push_back( gen2 ) ;

	std::cerr << "Creating files...\n" ;
	
	create_files( original, gen, gen2 ) ;

	genfile::SNPDataSourceChain chain ;
	std::auto_ptr< genfile::SNPDataSource > gen1source( genfile::SNPDataSource::create( gen )) ;
	std::auto_ptr< genfile::SNPDataSource > gen2source( genfile::SNPDataSource::create( gen2 )) ;
	chain.add_source( gen1source ) ;
	TEST_ASSERT( chain.number_of_samples() == data::number_of_samples ) ;
	chain.add_source( gen2source ) ;
	TEST_ASSERT( chain.number_of_samples() == data::number_of_samples ) ;

	std::vector< std::vector< SnpData > > results ;
	std::cerr << "Reading gen files (1)...\n" ;
	results.push_back( read_snp_data( chain )) ;
	std::cerr << "Reading gen files (2)...\n" ;
	results.push_back( read_gen_files( filenames )) ;

	for( std::size_t i = 0; i < results.size(); ++i ) {
		TEST_ASSERT( results[i].size() == (2*data::number_of_snps) ) ;
		TEST_ASSERT( results[i] == results[0] ) ;
	}
}

AUTO_TEST_SUITE_END()
