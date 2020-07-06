
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_WITHSNPDOSAGESCOHORTINDIVIDUALSOURCE_HPP
#define GENFILE_WITHSNPDOSAGESCOHORTINDIVIDUALSOURCE_HPP

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "config/config.hpp"
#if HAVE_EIGEN
#include <Eigen/Core>
#endif
#include "genfile/CohortIndividualSource.hpp"
#include "genfile/CompositeCohortIndividualSource.hpp"
#include "genfile/SNPDataSource.hpp"
#include "genfile/VariantIdentifyingDataTest.hpp"

namespace genfile {
	class PositionMatchesTest: public genfile::VariantIdentifyingDataTest
	{
	public:
		PositionMatchesTest( GenomePosition const& position ) ;
		bool operator()( VariantIdentifyingData const& ) const ;
 		std::string display() const ;
 	private:
		GenomePosition const m_position ;
	} ;

 	class WithSNPDosagesCohortIndividualSource: public genfile::CompositeCohortIndividualSource {
	public:
		typedef genfile::VariantIdentifyingDataTest SNPMatcher ;
		typedef std::auto_ptr< WithSNPDosagesCohortIndividualSource > UniquePtr ;
		typedef std::map< SNPMatcher::SharedPtr, std::set< std::string > > SNPDosageSpec ;
		typedef genfile::GenomePosition GenomePosition ;

		static UniquePtr create(
			CohortIndividualSource::ConstUniquePtr sample_source,
			genfile::SNPDataSource& snp_data_source,
			SNPDosageSpec const& snp_matchers
		) ;
		
		static VariantIdentifyingDataTest::UniquePtr create_snp_matcher( std::string test_spec, std::string const& splitter = "~" ) ;
		
	public:
		WithSNPDosagesCohortIndividualSource(
			genfile::CohortIndividualSource::ConstUniquePtr sample_source,
			genfile::SNPDataSource& snp_data_source,
			SNPDosageSpec const& snp_matchers
		) ;
		
		std::size_t get_number_of_individuals() const ;
		ColumnSpec get_column_spec() const ;

		Entry get_entry( std::size_t sample_i, std::string const& column_name ) const ;
		Entry get_entry( std::size_t sample_i, std::size_t const column_index ) const ;

		genfile::CohortIndividualSource const& get_parent_source() const ;
		genfile::CohortIndividualSource const& get_base_source() const ;

		std::string get_source_spec() const ;
	private:
		
		void setup( genfile::SNPDataSource& snp_data_source, SNPDosageSpec const& rsids ) ;
		void add_column( std::string const& column_name, Eigen::MatrixXd const& column ) ;
		
		CohortIndividualSource::ConstUniquePtr m_source ;
		std::vector< std::string > m_column_names ;
		std::vector< ColumnType > m_column_types ;
		std::vector< std::vector< Entry > > m_column_data ;
	} ;
}

#endif
