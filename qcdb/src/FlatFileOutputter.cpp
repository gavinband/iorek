
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <memory>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include "genfile/VariantEntry.hpp"
#include "genfile/VariantIdentifyingData.hpp"
#include "genfile/Error.hpp"
#include "genfile/string_utils.hpp"
#include "statfile/BuiltInTypeStatSink.hpp"
#include "appcontext/get_current_time_as_string.hpp"
#include "qcdb/Storage.hpp"
#include "qcdb/FlatFileOutputter.hpp"

namespace qcdb {
	namespace {
		void append_to_string( std::string* target, std::string const& value ) {
			(*target) += ( target->size() > 0 ? "," : "" ) + value ;
		}
	}

	FlatFileOutputter::UniquePtr FlatFileOutputter::create( std::string const& filename, std::string const& analysis_name, Metadata const& metadata ) {
		return UniquePtr( new FlatFileOutputter( filename, analysis_name, metadata ) ) ;
	}

	FlatFileOutputter::SharedPtr FlatFileOutputter::create_shared( std::string const& filename, std::string const& analysis_name, Metadata const& metadata ) {
		return SharedPtr( new FlatFileOutputter( filename, analysis_name, metadata ) ) ;
	}

	FlatFileOutputter::FlatFileOutputter( std::string const& filename, std::string const& analysis_name, Metadata const& metadata ):
		m_filename( filename ),
		m_analysis_name( analysis_name ),
		m_metadata( metadata ),
		m_max_snps_per_block( 1000 )
	{
		m_snps.reserve( 1000 ) ;
	}
	
	FlatFileOutputter::~FlatFileOutputter() {
	}
	
	void FlatFileOutputter::finalise( long ) {
		store_block() ;
		m_snps.clear() ;
		m_values.clear() ;
		m_sink->write_comment( "Completed successfully at " + appcontext::get_current_time_as_string() ) ;
	}

	FlatFileOutputter::AnalysisId FlatFileOutputter::analysis_id() const {
		// A flat file only ever has one analysis.
		return 0 ;
	}

	void FlatFileOutputter::add_variable(
		std::string const& variable
	) {
		VariableMap::left_const_iterator where = m_variables.left.find( variable ) ;
		if( where == m_variables.left.end() ) {
			if( m_sink.get() ) {
				// Uh-oh, have already written a header.
				throw genfile::BadArgumentError( "qcdb::FlatFileOutputter::add_variable()", "variable=\"" + variable + "\"" ) ;
			}
			else {
				// Still have time to add the variable to our list of variables, retaining the order of addition.
				where = m_variables.left.insert( VariableMap::left_value_type( variable, m_variables.size() ) ).first ;
			}
		}
	}

	void FlatFileOutputter::create_new_variant( genfile::VariantIdentifyingData const& snp ) {
		if( m_snps.size() == m_max_snps_per_block ) {
			store_block() ;
			m_snps.clear() ;
			m_values.clear() ;
		}
		m_snps.push_back( snp ) ;
	}

	void FlatFileOutputter::store_per_variant_data(
		genfile::VariantIdentifyingData const& snp,
		std::string const& variable,
		genfile::VariantEntry const& value
	) {
		bool const new_snp = m_snps.empty() || snp != m_snps.back() ;
		if( new_snp ) {
			// If we have a whole block's worth of data, store it now.
			if( m_snps.size() == m_max_snps_per_block ) {
				store_block() ;
				m_snps.clear() ;
				m_values.clear() ;
			}
			m_snps.push_back( snp ) ;
		}

		VariableMap::left_const_iterator where = m_variables.left.find( variable ) ;
		if( where == m_variables.left.end() ) {
			if( m_sink.get() ) {
				// Uh-oh, have already written a header.
				throw genfile::BadArgumentError( "qcdb::FlatFileOutputter::store_per_variant_data()", "variable=\"" + variable + "\"" ) ;
			}
			else {
				// Still have time to add the variable to our list of variables, retaining the order of addition.
				where = m_variables.left.insert( VariableMap::left_value_type( variable, m_variables.size() ) ).first ;
			}
		}

		// Store the value of this variable.
		m_values[ std::make_pair( m_snps.size() - 1, where->second ) ] = value ;
	}

	void FlatFileOutputter::store_block() {
		if( !m_sink.get() ) {
			m_sink = statfile::BuiltInTypeStatSink::open( m_filename ) ;
			m_sink->write_metadata( format_metadata() ) ;

			(*m_sink) | "alternate_ids" | "rsid" | "chromosome" | "position" | "alleleA" | "alleleB" ;
			VariableMap::right_const_iterator
				i = m_variables.right.begin(),
				end_i = m_variables.right.end() ;
			for( ; i != end_i; ++i ) {
				(*m_sink).add_column( i->second ) ;
			}
			(*m_sink) << statfile::begin_data() ;
		}
		for( std::size_t snp_i = 0; snp_i < m_snps.size(); ++snp_i ) {
			genfile::VariantIdentifyingData const& snp = m_snps[ snp_i ] ;
			std::string SNPID ;
			if( snp.number_of_identifiers() == 1 ) {
				SNPID = "NA" ;
			} else {
				snp.get_identifiers( boost::bind( &append_to_string, &SNPID, _1 ), 1 ) ;
			}
			(*m_sink) << SNPID << snp.get_primary_id() << snp.get_position().chromosome()
				<< snp.get_position().position()
				<< snp.get_allele(0)
				<< (( snp.number_of_alleles() < 2 ) ? "." : snp.get_alleles_as_string( ",", 1, snp.number_of_alleles() )) ;
			VariableMap::right_const_iterator
				var_i = m_variables.right.begin(),
				end_var_i = m_variables.right.end() ;
			for( ; var_i != end_var_i; ++var_i ) {
				ValueMap::const_iterator where = m_values.find( std::make_pair( snp_i, var_i->first )) ;
				if( where == m_values.end() ) {
					(*m_sink) << genfile::MissingValue() ;
				}
				else {
					(*m_sink) << where->second ;
				}
			}
			(*m_sink) << statfile::end_row() ;
		}
	}
	
	std::string FlatFileOutputter::format_metadata() const {
		std::ostringstream str ;
		str << "Analysis: \"" << m_analysis_name << "\"\n"
			<< " started: " << appcontext::get_current_time_as_string() << "\n" ;
		str << "\nAnalysis properties:\n" ;
		for( Metadata::const_iterator i = m_metadata.begin(); i != m_metadata.end(); ++i ) {
			str << "  "
				<< i->first
				<< " "
				<< genfile::string_utils::join( i->second.first, " " )
				<< " (" + i->second.second + ")\n" ;
		}
		return str.str() ;
	}
}
