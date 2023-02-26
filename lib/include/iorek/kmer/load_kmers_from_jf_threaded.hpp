
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOREK_LOAD_KMERS_FROM_JF_THREADED
#define IOREK_LOAD_KMERS_FROM_JF_THREADED

#include <iostream>
#include <mutex>
#include <cstdint>

#include "parallel_hashmap/phmap.h"

#define DEBUG 1

namespace iorek {
	namespace kmer {
		typedef phmap::parallel_flat_hash_set<
			uint64_t,
			phmap::priv::hash_default_hash<uint64_t>,
			phmap::priv::hash_default_eq<uint64_t>,
			phmap::priv::Allocator<uint64_t>,
			5, // 2^(this number) of submaps
			std::mutex
		> ParallelHashSet ;

		std::size_t load_kmers_from_jf_threaded(
			std::string const& jf_filename,
			ParallelHashSet* destination,
			std::size_t number_of_threads,
			std::function< void (std::size_t) > progress_callback,
			uint64_t lower_multiplicity_threshold = 0,
			std::size_t max_kmers = std::numeric_limits< std::size_t >::max(),
			bool verbose = false
		) ;
	}
}


#endif
