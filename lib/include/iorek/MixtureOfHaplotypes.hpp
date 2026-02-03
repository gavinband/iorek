
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_MIXTUREOFHAPLOTYPES_HPP
#define SVELTE_APPS_MIXTUREOFHAPLOTYPES_HPP

#include <vector>
#include <map>
#include <functional>

namespace iorek {
	struct MixtureOfHaplotypes {
		MixtureOfHaplotypes(
			std::vector< std::size_t > indices
		):
			m_indices( indices )
		{}

		void add( std::string const& haplotype, double const weight = 1.0 ) ;
		void remove( std::size_t i ) ;
		void split( std::size_t to_split, std::string const& haplotype, double const weight ) ;

		std::size_t const number_of_haplotypes() const { return m_haplotypes.size() ; }
		std::string const& haplotype( std::size_t i ) const { return m_haplotypes[i] ; }
		double const& weight( std::size_t i ) const { return m_weights[i] ; }
		bool contains( std::string const& haplotype ) const ;
		std::size_t find( std::string const& haplotype ) const ;

	private:
		std::vector< std::string > m_indices ;
		std::vector< double > m_weights ;
	} ;

}

#endif
