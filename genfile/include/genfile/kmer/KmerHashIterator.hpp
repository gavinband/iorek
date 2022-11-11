
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_KMER_KMERHASHITERATOR_HPP
#define GENFILE_KMER_KMERHASHITERATOR_HPP

#include <cstdint>
#include <string>
#include <cassert>
#include "genfile/string_utils/string_utils.hpp"
#include <iostream>

namespace genfile {
	namespace kmer {
		// Decode hash into a sequence.
		// This encoding matches that of jellyfish
		std::string decode_hash( uint64_t hash, std::size_t k ) {
			std::string result( k, ' ' ) ;
			//std::cerr << "++ decode_hash( " << std::hex << hash << std::dec << ", " << k << "):\n" ;
			
			for( std::size_t i = 0; i < k; ++i ) {
				uint8_t bits = (hash >> (2*(k-i-1))) & 0x3 ;
				switch( bits ) {
					case 0:
						result[i] = 'A' ;
						break ;
					case 3:
						result[i] = 'T' ;
						break ;
					case 1:
						result[i] = 'C' ;
						break ;
					case 2:
						result[i] = 'G' ;
						break ;
				}
				//std::cerr << "     i=" << i << ", bits = " << int(bits) << ", \"" << result[i] << "\"\n" ;
			}
			return result ;
		}
		
		uint8_t reverse_complement( uint8_t encoded ) {
			return (encoded < 4) ? (~encoded & 0x3) : 4u ;
		}
		
		uint64_t reverse_complement( uint64_t hash, std::size_t k ) {
			uint64_t result = 0 ;
			for( std::size_t i = 0; i < k; ++i ) {
				result <<= 2 ;
				result |= ((~hash) & 0x3) ;
				hash >>= 2 ;
			}
			return result ;
		}

		// KmerHashIterator iterates over all kmers in a sequence of A, C, G and T.
		// It encodes each kmer as a hash value inside a 64-bit integer
		// where A->00, C->01, G->10 and T->11
		// A total of 2k bits of the hash are used.
		// This encoding matches that of the mer_dna class of Jellyfish2.
		// The leftmost base occupies the most-significant 2 bits of the uint64 and so on.
		// The hash of the kmer and its reverse complement are simultaneously computed.
		template< typename Iterator >
		struct KmerHashIterator {
			KmerHashIterator( Iterator begin, Iterator const end, std::size_t k ):
				m_begin( begin ),
				m_end( end ),
				m_k( k ),
				m_cursor( 0 ),
				m_last_n_base( k ),
				m_forward_hash(0ul),
				m_reverse_complement_hash(0ul),
				m_mask( ~(0xFFFFFFFFFFFFFFFF << (m_k*2)))
			{
				assert( m_k < 33 ) ;
				// fill hashes with first k bases
				for( m_cursor = 0; m_cursor < m_k && m_begin != m_end; ++m_begin, ++m_cursor ) {
					m_forward_hash <<= 2 ;
					m_forward_hash &= m_mask ;
					m_reverse_complement_hash >>= 2 ;
					++m_last_n_base ;

					uint8_t const encoded = forward_encode_base( *m_begin ) ;
					m_last_n_base = (encoded < 4) ? m_last_n_base : 0 ;
					m_forward_hash |= uint64_t(encoded & 0x3) ;
					m_reverse_complement_hash |= uint64_t(
						(encoded < 4) ? reverse_complement( encoded ) : 0
					) << (2*m_k-2) ;
				}
			}

			KmerHashIterator& operator++() {
				// Update forward and reverse complement hashes with next base.
				m_forward_hash <<= 2 ;
				m_forward_hash &= m_mask ;
				m_reverse_complement_hash >>= 2 ;
				m_last_n_base += (m_last_n_base < m_k) ? 1 : 0 ;

				uint8_t const encoded = forward_encode_base( *m_begin ) ;
				m_last_n_base = (encoded < 4) ? m_last_n_base : 0 ;
				m_forward_hash |= (encoded & 0x3) ;
				m_reverse_complement_hash |= uint64_t(((encoded < 4) ? reverse_complement( encoded ) : 0)) << (m_k*2-2) ;

				++m_begin ;
				return *this ;
			}

			bool finished() const { return (m_begin == m_end) ; }
			bool filled() const { return m_cursor == m_k ; }
			bool contains_error_bases() const { return m_last_n_base < m_k ; }
			uint64_t hash() const { return m_forward_hash ; }
			uint64_t reverse_complement_hash() const { return m_reverse_complement_hash ; }
			uint64_t minimum_hash() const {
				return ( m_forward_hash < m_reverse_complement_hash
					? m_forward_hash
					: m_reverse_complement_hash
				) ;
			}
			
			std::string to_string() const {
				using genfile::string_utils::to_string ;
				return "kmer ("
					+ to_string( m_cursor )
					+  "/" + to_string( m_k )
					+ "/" + to_string( m_end - m_begin )
					+ "): "
					+ decode_hash( m_forward_hash, m_k )
					+ " / " + decode_hash( m_reverse_complement_hash, m_k )
					+ " / " + decode_hash( minimum_hash(), m_k ) ;
			}

		private:
			Iterator m_begin ;
			Iterator const m_end ;
			std::size_t m_k ;
			std::size_t m_cursor ;
			std::size_t m_last_n_base ;
			uint64_t m_forward_hash ;
			uint64_t m_reverse_complement_hash ;
			uint64_t m_mask ;
			
		private:
			uint8_t forward_encode_base( char const base ) const {
				switch( base ) {
					// Important: complementary bases are bit-inverses of each other.
					// Important: this encoding matches that of jellyfish.
					case 'a':
					case 'A':
						return 0u ; // 00
					case 't':
					case 'T':
						return 3u ; // 11
					case 'c':
					case 'C':
						return 1u ; // 01
					case 'g':
					case 'G':
						return 2u ; // 10
					default:
						// We use 4 to denote an unrecognised base.
						return 4u ;
				}
			}
		} ;
	}
}

#endif // GENFILE_KMER_KMERHASHITERATOR_HPP
