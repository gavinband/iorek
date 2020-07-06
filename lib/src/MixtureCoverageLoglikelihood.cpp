
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)


#include <Eigen/Core>
#include "boost/unordered_map.hpp"
#include "svelte/HalfOpenRange.hpp"
#include "svelte/HalfOpenRangeLevel.hpp"
#include "svelte/CoverageProfile.hpp"
#include "svelte/GaussianCoverageLoglikelihood.hpp"
#include "svelte/SVPopulationModel.hpp"
#include "svelte/MixtureCoverageLoglikelihood.hpp"

// #define DEBUG 1
#if DEBUG
#include <iostream>
#endif

namespace svelte {
	MixtureCoverageLoglikelihood::MixtureCoverageLoglikelihood(
		Eigen::VectorXd const& means,
		Eigen::VectorXd const& variances,
		Eigen::MatrixXd const& coverage
	):
		m_ll( means, variances, coverage )
	{}

	void MixtureCoverageLoglikelihood::evaluate( SVPopulationModel const& model, Eigen::MatrixXd* result ) const {
		std::size_t const P = model.profiles().size() ;
		std::size_t const number_of_models = P*(P+1)/2 ;
		std::size_t const N = m_ll.number_of_samples() ;

#if DEBUG
		std::cerr << "MixtureCoverageLoglikelihood::evaluate(): model: " << model.toString() << ".\n" ;
#endif
		
		result->setZero( N, number_of_models ) ;
		std::vector< double > frequencies( number_of_models, 0.0 ) ;
		int count = 0 ;
		model.generate_diploid_profiles(
			[&result,this,&count,&frequencies]( CoverageProfile const& profile, double const weight ) {
#if DEBUG
				std::cerr << "MixtureCoverageLoglikelihood::evaluate(): profile: " << profile.toString() << ", weight = " << weight << ".\n" ;
#endif
				m_ll.evaluate( profile, result->col( count ) ) ;
				frequencies[count] = weight ;
				++count ;
			}
		) ;
		auto f = Eigen::Map< Eigen::MatrixXd const >( frequencies.data(), 1, frequencies.size() ) ;
#if DEBUG
		std::cerr << "Frequencies = " << f << ".\n" ;
#endif
		result->array() += f.array().log().replicate( N, 1 ) ;
	}
}
