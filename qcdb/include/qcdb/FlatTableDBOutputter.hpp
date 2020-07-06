
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef QCTOOL_QCDB_FLAT_TABLE_DB_OUTPUTTER_HPP
#define QCTOOL_QCDB_FLAT_TABLE_DB_OUTPUTTER_HPP

#include <string>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bimap.hpp>
#include "genfile/VariantIdentifyingData.hpp"
#include "genfile/CohortIndividualSource.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/db/Connection.hpp"
#include "genfile/db/SQLStatement.hpp"
#include "qcdb/DBOutputter.hpp"
#include "qcdb/Storage.hpp"

namespace qcdb {
	struct FlatTableDBOutputter: public Storage {
		typedef std::auto_ptr< FlatTableDBOutputter > UniquePtr ;
		typedef boost::shared_ptr< FlatTableDBOutputter > SharedPtr ;
		typedef DBOutputter::Metadata Metadata ;
		static UniquePtr create(
			std::string const& filename,
			std::string const& cohort_name,
			std::string const& analysis_description,
			Metadata const& metadata,
			std::string const& snp_match_fields = "position,alleles",
			boost::optional< genfile::db::Connection::RowId > analysis_id = boost::optional< genfile::db::Connection::RowId >()
		 ) ;
		static SharedPtr create_shared(
			std::string const& filename,
			std::string const& cohort_name,
			std::string const& analysis_description,
			Metadata const& metadata,
			std::string const& snp_match_fields = "position,alleles",
			boost::optional< genfile::db::Connection::RowId > analysis_id = boost::optional< genfile::db::Connection::RowId >()
		) ;

		FlatTableDBOutputter(
			std::string const& filename,
			std::string const& analysis_name,
			std::string const& analysis_description,
			Metadata const& metadata,
			std::string const& snp_match_fields = "position,alleles",
			boost::optional< genfile::db::Connection::RowId > analysis_id = boost::optional< genfile::db::Connection::RowId >()
		) ;

		~FlatTableDBOutputter() ;

		void set_table_name( std::string const& table_name ) ;
		
		void add_variable(
			std::string const& 
		) ;
		
		void create_new_variant( genfile::VariantIdentifyingData const& ) ;
		void store_per_variant_data(
			genfile::VariantIdentifyingData const& snp,
			std::string const& variable,
			genfile::VariantEntry const& value
		) ;
		
		void finalise( long options = eCreateIndices ) ;

		AnalysisId analysis_id() const ;

	private:
		DBOutputter m_outputter ;
		std::string m_table_name ;
		std::size_t const m_max_snps_per_block ;
		genfile::db::Connection::StatementPtr m_insert_data_sql ;
		std::vector< genfile::VariantIdentifyingData > m_snps ;
		typedef boost::bimap< std::string, std::size_t > VariableMap ;
		VariableMap m_variables ;
		typedef std::map< std::pair< std::size_t, std::size_t >, genfile::VariantEntry > ValueMap ;
		ValueMap m_values ;

	private:
		std::string get_table_name() const ;
		void store_block() ;
		void create_schema() ;
		void store_data_for_variant(
			std::size_t const,
			genfile::VariantIdentifyingData const& snp,
			genfile::db::Connection::RowId const,
			genfile::db::Connection::RowId const
		) ;
		void create_variables() ;
	} ;
}

#endif
