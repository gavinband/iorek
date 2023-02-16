
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_MIXTURECOVERAGELOGLIKELIHOOD_HPP
#define SVELTE_APPS_MIXTURECOVERAGELOGLIKELIHOOD_HPP

#include <map>
#include <Eigen/Core>
#include <boost/unordered_map.hpp>
#include "iorek/HalfOpenRange.hpp"
#include "iorek/HalfOpenRangeLevel.hpp"
#include "iorek/CoverageProfile.hpp"
#include "iorek/GaussianCoverageLoglikelihood.hpp"
#include "iorek/SVPopulationModel.hpp"

namespace iorek {
	struct MixtureCoverageLoglikelihood {
		typedef Eigen::MatrixXd Matrix ;
		typedef Eigen::VectorXd Vector ;
		typedef Eigen::RowVectorXd RowVector ;

		MixtureCoverageLoglikelihood(
			Eigen::VectorXd const& means,
			Eigen::VectorXd const& variances,
			Eigen::MatrixXd const& coverage
		) ;

		// Evaluate and return a matrix reflecting the loglikelihood for each
		// diploid coverage profile (columns) for each sample (rows).
		void evaluate( SVPopulationModel const& model, Matrix* result ) const ;

	private:
		GaussianCoverageLoglikelihood const m_ll ;
		typedef std::map< CoverageProfile, Eigen::VectorXd > Cache ;
		mutable Cache m_cache ;
	} ;
}

#endif
