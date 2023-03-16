
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <mutex>
#include <cstdint>

#include "jellyfish/jellyfish.hpp"
#include "parallel_hashmap/phmap.h"
#include "concurrentqueue/concurrentqueue.h"

#include "genfile/Error.hpp"
#include "iorek/kmer/load_kmers_from_jf_threaded.hpp"

#define DEBUG 1

namespace iorek {
	namespace kmer {
		namespace {
			typedef moodycamel::ConcurrentQueue< uint64_t > Queue ;

			// Worker thread function.
			// This simply reads kmers from the given queue, and inserts into the given hash set.
			// This function does not try to ensure thread-safety, i.e. it must be established
			// elsewhere that this thread won't write to component hash sets that other threads
			// are also writing to.
			void consume_and_insert_kmers(
				std::size_t const k,
				Queue* queue,
				ParallelHashSet* result,
				std::size_t const thread_index,
				std::atomic< int >* quit
			) {
				std::size_t count = 0 ;
				uint64_t elt ;
				while( !(*quit) ) {
					bool popped = queue->try_dequeue( elt ) ;
					if( popped ) {
						result->insert( elt ) ;
						++count ;
					} else {
						// nothing to pop, sleep to allow queue to fill.
						std::this_thread::sleep_for( std::chrono::microseconds(10) ) ;
					}
				}
			}

			// Main dispatcher function.
			// This function reads kmers from the jellyfish file reader, decides which sub-index hash set they will
			// end up in, and puts the kmers in the appropriate queue.
			// The queues must have already been constructed.
			void read_kmers_into_queues(
				unsigned int const k,
				uint64_t const multiplicity_threshold,
				binary_reader& reader,
				std::vector< Queue >& queues,
				ParallelHashSet const& set,
				std::function< void( std::size_t ) > progress_callback,
				std::size_t const max_kmers = std::numeric_limits< std::size_t >::max()
			) {
				jellyfish::mer_dna::k( k ) ;
				std::size_t count = 0 ;
				{
					progress_callback(0) ;
					while( reader.next() && (count < max_kmers) ) {
						if( reader.val() >= multiplicity_threshold ) {
							uint64_t const kmer = reader.key().get_bits( 0, 2*k ) ;

							// Figure out which thread this kmer should be sent to 
							std::size_t const hashvalue = set.hash( kmer ) ;
							std::size_t idx = set.subidx( hashvalue ) ;
	#if DEBUG > 1
							std::cerr << "++ kmer: " << genfile::kmer::decode_hash( kmer, k )
								<< ", "
								<< std::hex << kmer << std::dec
								<< ": " << "hashvalue: "
								<< hashvalue << ", idx: " << idx << ".\n" ;
	#endif

							// The subidx will be in the range 0...31,
							// but we may have a smaller number of actual worker threads.
							// We therefore choose the thread with index modulo the number of threads.
							std::size_t const queue_index = idx % queues.size() ;
							// Queue& queue = queues[ queue_index ] ;
							Queue& queue = queues[ queue_index ] ;
							while( !queue.try_enqueue( kmer )) {
	#if DEBUG > 1
								std::cerr << "-- queue full after " << count << " kmers, sleeping...\n" ;
	#endif
								std::this_thread::sleep_for( std::chrono::microseconds(10) ) ;
							}
	#if DEBUG > 1
							std::cerr << "++ Wrote " << kmer << " to queue " << queue_index << ".\n" ;
	#endif
							progress_callback( ++count ) ;
						}
					}
				}
			}
		}

		std::size_t load_kmers_from_jf_threaded(
			std::string const& filename,
			ParallelHashSet* result,
			std::size_t number_of_threads,
			std::function< void (std::size_t) > progress_callback,
			uint64_t const multiplicity_threshold,
			std::size_t const max_kmers
		) {
			std::ifstream ifs( filename ) ;
			jellyfish::file_header header( ifs ) ;
			std::size_t const k = header.key_len() / 2 ;

			if( header.format() != binary_dumper::format ) {
				throw genfile::BadArgumentError(
					"iorek::kmer::load_kmers_from_jf_threaded()",
					"filename=\"" + filename + "\"",
					"Expected a binary-format jellyfish count file."
				) ;
			}

			// Check threads are a sensible power of 2.
			std::cerr << "THREADS: " << number_of_threads << ".\n" ;
			assert( number_of_threads >= 1 ) ;
			assert( number_of_threads <= 32 ) ;
			assert( (number_of_threads & (number_of_threads - 1) ) == 0 ) ;
			
			jellyfish::mer_dna::k( k ) ;
			binary_reader reader(ifs, &header);

			/// @brief  The parallel hash map used here works by holding a set of 2^n hashmaps
			// internally, so that inserts into each component hash set can be concurrent.
			// The 'subidx()' method can be used to tell which component hash set each key will
			// be inserted into.

			// To leverage this, we set up a power-of-two number of worker threads whose
			// job is to insert kmers into the hash set.  Each thread has its own queue
			// which contains kmers to insert.  In this thread, we check which subidx each kmer
			// corresponds to, then send it to the appropriate thread for insertion.
			
			// In this way, the worker threads are able to run without synchronisation and hopefully
			// maximise the insertion throughput.

			{
				std::vector< Queue > queues ;
				std::vector< std::thread > threads ;
				std::atomic< int > quit(0) ;

#if DEBUG
				std::cerr << "++ Loading kmers from \"" << filename << "\"\n"
					<< "   ...using " << number_of_threads << " worker threads...\n" ;
#endif		
				// Create worker thread queues
				queues.reserve( number_of_threads ) ;
				for( std::size_t i = 0; i < number_of_threads; ++i ) {
					queues.push_back( Queue( 32768 ) ) ;
				}
				// Create worker threads
				threads.reserve( number_of_threads ) ;
				for( std::size_t i = 0; i < number_of_threads; ++i ) {
					threads.push_back(
						std::thread(
							consume_and_insert_kmers,
							k,
							&(queues[i]),
							result,
							i,
							&quit
						)
					) ;
				}
				
				// Read kmers into queues.
				read_kmers_into_queues(
					k,
					multiplicity_threshold,
					reader,
					queues,
					*result,
					progress_callback,
					max_kmers
				) ;
				
				// Wait for workers to finish.
				for( std::size_t i = 0; i < number_of_threads; ++i ) {
					while( queues[i].size_approx() > 0 ) {
						std::this_thread::sleep_for( std::chrono::milliseconds(1)) ;
					}
				}

				quit = 1 ;
				std::this_thread::sleep_for( std::chrono::milliseconds(1)) ;
				for( std::size_t i = 0; i < number_of_threads; ++i ) {
					threads[i].join() ;
				}
			}
			return k ;
		}
	}
}

