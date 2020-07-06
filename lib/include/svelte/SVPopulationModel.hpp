
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_SVPOPULATIONMODEL_HPP
#define SVELTE_APPS_SVPOPULATIONMODEL_HPP

#include <vector>
#include <map>
#include <functional>
#include "svelte/HalfOpenRange.hpp"
#include "svelte/HalfOpenRangeLevel.hpp"
#include "svelte/CoverageProfile.hpp"

namespace svelte {
	// An SVPopulationModel consists of a set of coverage profiles (to be thought
	// of as profiles from individual haplotypes)
	// together with frequencies summing to 1.
	// E.g. a diploid coverage profile can then be selected by summing two of
	// these profiles drawn with probability equal to the frequency.
	struct SVPopulationModel {
	public:
		SVPopulationModel( uint32_t start, uint32_t end ):
			m_profiles( 1, CoverageProfile( start, end, 1 )),
			m_frequencies( 1, 1.0 )
		{
			m_index[ m_profiles[0] ] = 0 ;
		}

		SVPopulationModel( CoverageProfile const& profile ):
			m_profiles( 1, profile ),
			m_frequencies( 1, 1.0 )
		{
			m_index[ m_profiles[0] ] = 0 ;
		}

		SVPopulationModel( SVPopulationModel const& other ):
			m_profiles( other.m_profiles ),
			m_index( other.m_index ),
			m_frequencies( other.m_frequencies )
		{
		}

		SVPopulationModel& operator=( SVPopulationModel const& other ) {
			m_profiles = other.m_profiles ;
			m_index = other.m_index ;
			m_frequencies = other.m_frequencies ;
			return *this ;
		}

		std::string toString() const ;
		
		std::vector< CoverageProfile > const& profiles() const { return m_profiles ; }
		std::vector< double > const& frequencies() const { return m_frequencies ; }

	public:		
		void add( CoverageProfile const& profile, double weight ) ;
		void remove( CoverageProfile const& profile ) ;
		void replace( std::size_t i, CoverageProfile const& profile ) ;
		bool contains( CoverageProfile const& profile ) const ;

		void generate_diploid_profiles( std::function< void( CoverageProfile const& profile, double const weight ) > ) const ;

	private:
		// TODO: perhaps rewrite as std::vector< std::pair< CoverageProfile, double > > ?
		// list of coverage profiles, one per frequency
		std::vector< CoverageProfile > m_profiles ;
		// structure to allow lookup of profile
		std::map< CoverageProfile, std::size_t > m_index ;
		// modelled frequency of each profile, summing to one
		std::vector< double > m_frequencies ;
		// invariant: frequencies always sum to 1.
		// invariant: m_profiles, m_index and m_frequencies always have the same size
		// invariant: m_profiles never contains the same index twice.
	} ;
}

#endif
