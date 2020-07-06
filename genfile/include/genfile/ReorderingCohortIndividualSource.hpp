
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_REORDERING_COHORTINDIVIDUALSOURCE_HPP

#include <string>
#include <memory>
#include <iosfwd>
#include <set>
#include <algorithm>

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "genfile/Error.hpp"
#include "genfile/string_utils.hpp"
#include "genfile/MissingValue.hpp"
#include "genfile/VariantEntry.hpp"

namespace genfile {
	/*
	* Present a view of the sample information with samples reordered according to the 
	* second argument.
	* Argument 'order' is a mapping from target to source samples
	* i.e. if order = (v_j), j = 1...N, then v_j is the index of target sample
	* j in the source data.
	*/
	class ReorderingCohortIndividualSource: public CohortIndividualSource
	{
	public:
		ReorderingCohortIndividualSource(
			CohortIndividualSource::UniquePtr source,
			std::vector< std::size_t > const& order
		): 
			m_source( source ),
			m_order( order )
		{
			assert( m_source.get() ) ;
			assert( m_order.size() == m_source->size() ) ;
			assert( *std::min_element( m_order.begin(), m_order.end() ) == 0 ) ;
			assert( *std::max_element( m_order.begin(), m_order.end() ) == ( m_order.size() - 1 ) ) ;
			assert( std::set< std::size_t > ( m_order.begin(), m_order.end() ).size() == m_order.size() ) ;
		}
		
		std::size_t get_number_of_individuals() const {
			return m_source->get_number_of_individuals() ;
		}

		ColumnSpec get_column_spec() const {
			return m_source->get_column_spec() ;
		}

		bool check_for_column( std::string const& column_name ) const {
			return m_source->check_for_column( column_name ) ;
		}

		Entry get_entry( std::size_t sample_i, std::string const& column_name ) const {
			std::size_t original_sample_i = m_order[ sample_i ] ;
			return m_source->get_entry( original_sample_i, column_name ) ;
		}

		Entry get_entry( std::size_t sample_i, std::size_t const column_index ) const {
			std::size_t original_sample_i = m_order[ sample_i ] ;
			return m_source->get_entry( original_sample_i, column_index ) ;
		}

		CohortIndividualSource const& get_base_source() const {
			return m_source->get_base_source() ;
		}
		CohortIndividualSource const& get_parent_source() const {
			return *m_source ;
		}

		// method: get_source_spec()
		// get_source_spec() returns a human-readable specification for this source.
		std::string get_source_spec() const {
			return "reordered:" + m_source->get_source_spec() ;
		}
	private:
		
		CohortIndividualSource::UniquePtr m_source ;
		std::vector< std::size_t > m_order ;
	} ;
}

#endif
