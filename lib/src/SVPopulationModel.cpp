
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <sstream>
#include <vector>
#include "svelte/HalfOpenRange.hpp"
#include "svelte/HalfOpenRangeLevel.hpp"
#include "svelte/CoverageProfile.hpp"
#include "svelte/SVPopulationModel.hpp"

namespace svelte {
	
	std::string SVPopulationModel::toString() const {
		std::ostringstream str ;
		str.precision(5) ;
		str << "{" ;
		for( std::size_t i = 0; i < m_profiles.size(); ++i ) {
			str << ((i>0) ? "|" : "" )
				<< m_profiles[i].toString() << ":" << m_frequencies[i] ;
		}
		str << "}" ;
		return str.str() ;
	}
	
	void SVPopulationModel::add( CoverageProfile const& profile, double weight ) {
		assert( weight >= 0.0 && weight <= 1.0 ) ;
		assert( m_index.find( profile ) == m_index.end() ) ;

		// scale existing frequencies down proportionally.
		for( auto &f: m_frequencies ) {
			f *= ( 1.0 - weight ) ;
		}
		m_profiles.push_back( profile ) ;
		m_frequencies.push_back( weight ) ;
		m_index[ profile ] = m_profiles.size() - 1 ;
	}

	void SVPopulationModel::remove( CoverageProfile const& profile ) {
		auto where = m_index.find( profile ) ;
		assert( where != m_index.end() ) ;
		std::size_t i = where->second ;
		double const weight = m_frequencies[i] ;
		// scale existing frequencies up proportionally.
		for( auto &f: m_frequencies ) {
			f /= ( 1.0 - weight ) ;
		}
		m_profiles.erase( m_profiles.begin() + i ) ;
		m_frequencies.erase( m_frequencies.begin() + i ) ;
		m_index.erase( where ) ;
	}

	void SVPopulationModel::replace( std::size_t i, CoverageProfile const& profile ) {
		assert( i > 0 && i < m_profiles.size() ) ;
		auto where = m_index.find( profile ) ;
		assert( where == m_index.end() ) ;

		m_index.erase( m_profiles[i] ) ;
		m_profiles[i] = profile ;
		m_index[ profile ] = i ;
	}
	
	bool SVPopulationModel::contains( CoverageProfile const& profile ) const {
		return m_index.find( profile ) != m_index.end() ;
	}
	
	void SVPopulationModel::generate_diploid_profiles(
		std::function< void( CoverageProfile const& profile, double const weight ) > callback
	) const {
		std::size_t const N = m_profiles.size() ;
		for( std::size_t i = 0; i < N; ++i ) {
			for( std::size_t j = i; j < N; ++j ) {
				callback(
					m_profiles[i] + m_profiles[j],
					m_frequencies[i] * m_frequencies[j] * ( 1 + (i != j) )
				) ;
			}
		}
	}
}

