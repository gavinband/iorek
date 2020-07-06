
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_VALUE_MAPPING_COHORT_INDIVIDUAL_SOURCE_HPP
#define GENFILE_VALUE_MAPPING_COHORT_INDIVIDUAL_SOURCE_HPP

#include "genfile/CohortIndividualSource.hpp"
#include "genfile/CrossCohortCovariateValueMapping.hpp"

namespace genfile {
	class ValueMappingCohortIndividualSource: public CohortIndividualSource
	{
	public:
		typedef std::auto_ptr< ValueMappingCohortIndividualSource > UniquePtr ;

	public:
		ValueMappingCohortIndividualSource(
			CohortIndividualSource::UniquePtr source
		) ;
		
		~ValueMappingCohortIndividualSource() ;
	
		void add_mapping(
			std::string const& source_column_name,
			std::string const& target_column_name,
			CrossCohortCovariateValueMapping::UniquePtr mapping
		) ;
		
		std::size_t get_number_of_individuals() const { return m_source->get_number_of_individuals() ; }
		ColumnSpec get_column_spec() const ;

		Entry get_entry( std::size_t sample_i, std::string const& column_name ) const ;
		Entry get_entry( std::size_t sample_i, std::size_t const column_index ) const ;

		std::string get_source_spec() const ;

		std::string get_summary( std::string const& prefix = "" ) const ;
	private:
		CohortIndividualSource::UniquePtr m_source ;
		typedef std::map< std::string, std::pair< std::string, CrossCohortCovariateValueMapping const* > > Mappings ;
		Mappings m_mappings ;
	} ;
}

#endif
