//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_GFA_HPP
#define GENFILE_GFA_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/optional.hpp>
#include "genfile/VariantEntry.hpp"

namespace genfile {
	namespace gfa {
		enum Orientation { ePlus = '+', eMinus = '-' } ;
		enum HaplotypeStatus { eNonBinnable = 'a', eHap1Specific = 'p', eHap2Specific = 'm' } ;

		struct TaggedElt {
			public:
				TaggedElt( std::size_t index ) ;

				std::size_t index() const { return m_index ; }

				void add_tag(
					std::string const& name,
					std::string const& type,
					std::string const& value
				) ;

			private:
				std::size_t m_index ;
				std::map< std::string, VariantEntry > m_tags ;
		} ;

		struct Header {
		} ;

		struct Segment: public TaggedElt {
			Segment(
				std::size_t index,
				std::string const& name,
				boost::optional< std::string > const& sequence
			) ;

			std::string const& name() const { return m_name ; }
			std::string const& sequence() const { return *m_sequence ; }

		private:
			std::string m_name ;
			boost::optional< std::string > m_sequence ;
		} ;

		struct Link: public TaggedElt {
			Link(
				std::size_t index,
				std::size_t const& from,
				Orientation const& from_orient,
				std::size_t const& to,
				Orientation const& to_orient,
				boost::optional< std::string > const& overlap_cigar
			) ;

			std::size_t const& from() const { return m_from ; }
			Orientation const& from_orient() const { return m_from_orient ; }
			std::size_t const& to() const { return m_to ; }
			Orientation const& to_orient() const { return m_to_orient ; }
			boost::optional< std::string > const& overlap_cigar() const { return m_overlap_cigar ; }

			private:
				std::size_t m_from ;
				Orientation m_from_orient ;
				std::size_t m_to ;
				Orientation m_to_orient ;
				boost::optional< std::string > m_overlap_cigar ;
		} ;

		// Hifiasm-specific A records
		struct ALine: public TaggedElt {
			ALine(
				std::size_t index,
				std::size_t contig_id,
				uint32_t contig_start,
				Orientation strand,
				std::string read_name,
				uint32_t read_start,
				uint32_t read_end,
				int read_id,
				HaplotypeStatus status
			) ;

			std::size_t contig_id() const { return m_contig_id ; }
			uint32_t contig_start() const { return m_contig_start ; }
			std::string const& read_name() const { return m_read_name ; }
			uint32_t read_start() const { return m_read_start ; }
			uint32_t read_end() const { return m_read_end ; }
			int read_id() const { return m_read_id ; }
			HaplotypeStatus haplotype_status() const { return m_status ; }

			private:
				std::size_t m_contig_id ;
				uint32_t m_contig_start ;
				Orientation m_strand ;
				std::string m_read_name ;
				uint32_t m_read_start ;
				uint32_t m_read_end ;
				int m_read_id ;
				HaplotypeStatus m_status ;
		} ;

		struct GFA {
		public:
			typedef std::unique_ptr< GFA > UniquePtr ;

			static UniquePtr create( std::istream& input ) ;

			std::size_t number_of_segments() const { return m_segments.size() ; }
			Segment const& segment( std::size_t i ) { return m_segments[i] ; }

			std::size_t number_of_links() const { return m_links.size() ; }
			Link const& link( std::size_t i ) { return m_links[i] ; }

		private:
			GFA( std::istream& input ) ;

			std::vector< Segment > m_segments ;
			std::map< std::string, std::size_t > m_segment_indices ;
			std::vector< Link > m_links ;
			std::map< std::size_t, std::vector< std::size_t > > m_links_from ;
			std::map< std::size_t, std::vector< std::size_t > > m_links_to ;
			std::vector< ALine > m_alines ;
			std::map< std::size_t, std::vector< std::size_t > > m_alines_for ;
		} ;
	}
}

#endif

