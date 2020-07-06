
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef SVELTE_APPS_GAUSSIANCOVERAGELOGLIKELIHOOD_HPP
#define SVELTE_APPS_GAUSSIANCOVERAGELOGLIKELIHOOD_HPP

#include <Eigen/Core>
#include "svelte/HalfOpenRange.hpp"
#include "svelte/HalfOpenRangeLevel.hpp"
#include "svelte/CoverageProfile.hpp"

namespace svelte {

	struct GaussianCoverageLoglikelihood {
	public:
		typedef Eigen::MatrixXd Matrix ;
		typedef Eigen::VectorXd Vector ;
		typedef Eigen::RowVectorXd RowVector ;
		typedef Eigen::Block< Matrix const > ConstMatrixBlock ;	
		typedef Eigen::VectorBlock< Vector const > ConstVectorSegment ;	

		GaussianCoverageLoglikelihood(
			Eigen::VectorXd const& means,
			Eigen::VectorXd const& variances,
			Eigen::MatrixXd const& coverage
		) ;

	public:
		void evaluate(
			CoverageProfile const& profile,
			Eigen::Ref< Vector > result
		) const ;

		int const number_of_sites() const { return m_coverage.rows() ; }
		int const number_of_samples() const { return m_coverage.cols() ; }
		
	private:
		// haploid expected coverage.
		Eigen::RowVectorXd const m_means ; 
		// haploid expected variance in coverage.
		Eigen::RowVectorXd const m_variances ; 
		// Observed coverage per-site (rows) and per-sample (columns)
		Eigen::MatrixXd const m_coverage ;
		// per-site scaling of coverage due to sequencing context
		Eigen::VectorXd m_site_factor ;
		// per-site propn of haploid coverage mismapping here
		Eigen::VectorXd m_mismap_propn ; 
		// temporary storage for actual variance 
		mutable Eigen::MatrixXd m_tmpVariances ;
		mutable Eigen::MatrixXd m_tmpA ;
		Eigen::MatrixXd m_precomputed_site_lls ;

	private:
		void precompute_site_lls() ;
		void accumulate(
			HalfOpenRangeLevel const& profilePoint,
			Eigen::Ref< Vector > result
		) const ;
	} ;
}

#endif
