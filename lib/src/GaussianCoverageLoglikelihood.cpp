
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <Eigen/Core>
#include "iorek/HalfOpenRange.hpp"
#include "iorek/HalfOpenRangeLevel.hpp"
#include "iorek/CoverageProfile.hpp"
#include "iorek/GaussianCoverageLoglikelihood.hpp"

// #define DEBUG 1
#if DEBUG
#include <iostream>
#endif

namespace iorek {

	GaussianCoverageLoglikelihood::GaussianCoverageLoglikelihood(
		Eigen::VectorXd const& means,
		Eigen::VectorXd const& variances,
		Eigen::MatrixXd const& coverage
	):
		m_means( means ),
		m_variances( variances ),
		m_coverage( coverage ),
		m_site_factor( Eigen::VectorXd::Constant( m_coverage.rows(), 1.0 )),
		m_mismap_propn( Eigen::VectorXd::Constant( m_coverage.rows(), 0.01 ))
	{
		assert( m_means.size() == m_variances.size() ) ;
		assert( m_means.size() == m_coverage.cols() ) ;
		precompute_site_lls() ;
	}
	
	void GaussianCoverageLoglikelihood::precompute_site_lls() {
		int const N = m_coverage.cols() ;
		int const L = m_coverage.rows() ;
		int const max_copy_number = 10 ;
		auto coverageBlock = m_coverage.block( 0, 0, L, N ) ;
		auto siteFactorBlock = m_site_factor.segment( 0, L ) ;
		auto mismapBlock = m_mismap_propn.segment( 0, L ) ;
		m_precomputed_site_lls.setZero( max_copy_number * L, N ) ;

		for( int copy_number = 0; copy_number < max_copy_number; ++copy_number ) {
			m_tmpVariances = (
				// variance from correctly mapped reads
				( ( m_variances * copy_number ).replicate( L, 1 ).array() ) * m_site_factor.replicate( 1, N ).array()
				// variance from mismapped reads
				+ ( m_variances.replicate( L, 1 ).array() * m_mismap_propn.replicate( 1, N ).array() )
			) ;
			
			m_precomputed_site_lls.block( copy_number * L, 0, L, N ) = -(
				0.5 * std::log( 2 * 3.141592653589793238462643383279502884L )
				+ m_tmpVariances.array().sqrt().log()
				+ 0.5 * (
					(
						coverageBlock.array() -
						(
							// mean from correctly mapped reads
							( (m_means * copy_number ).replicate( L, 1 ).array() * siteFactorBlock.replicate( 1, N ).array() )
							// mean from mismapped reads
							+ ( m_means.replicate( L, 1 ).array() * mismapBlock.replicate( 1, N ).array() )
						)
					).array().square()
					/ m_tmpVariances.array()
				)
			) ;
		}
	}

	void GaussianCoverageLoglikelihood::evaluate( CoverageProfile const& profile, Eigen::Ref< Vector > result ) const {
		assert( result.size() == m_means.size() ) ;
		result.setZero() ;
		for( auto profilePoint: profile ) {
			accumulate( profilePoint, result ) ;
		}
	}

	void GaussianCoverageLoglikelihood::accumulate(
		HalfOpenRangeLevel const& profilePoint,
		Eigen::Ref< Vector > result
	) const {
		int const N = m_coverage.cols() ;
		int const L = profilePoint.size() ;
		int const copy_number = profilePoint.level() ;
		assert( copy_number < 10 ) ;
		auto site_ll_block = m_precomputed_site_lls.block(
			copy_number * m_coverage.rows() + profilePoint.start(),
			0,
			L,
			N
		) ;
		result += site_ll_block.colwise().sum() ;
	}
}
