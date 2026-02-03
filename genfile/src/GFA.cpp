//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <map>
#include <boost/optional.hpp>
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/string_utils/slice.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/GFA.hpp"
#include "genfile/Error.hpp"

namespace genfile {
	namespace gfa {
		namespace {
			Orientation parse_orientation( std::string const& o ) {
				assert( o.size() == 1 ) ;
				Orientation result ;
				if( o[0] == '+' ) {
					result = ePlus ;
				} else if( o[0] == '-' ) {
					result = eMinus ;
				} else {
					throw genfile::BadArgumentError(
						"genfile::gfa::parse_orientation()",
						"o=\"" + o +"\"",
						"Unrecognised orientation must be + or -"
					) ;
				}
				return result ;
			}
			HaplotypeStatus parse_haplotype_status( std::string const& spec ) {
				HaplotypeStatus result ;
				if( spec[5] == 'a' ) {
					result = eNonBinnable ;
				} else if( spec[5] == 'p' ) {
					result = eHap1Specific ;
				} else if( spec[5] == 'm' ) {
					result = eHap2Specific ;
				} else {
					throw genfile::BadArgumentError(
						"genfile::gfa::parse_haplotype_status()",
						"spec=\"" + spec +"\"",
						"Expected HG:A:(a|p|m), found \"" + spec + "\""
					) ;
				}
				return result ;
			}

		}
		TaggedElt::TaggedElt( std::size_t index ):
			m_index( index )
		{}

		void TaggedElt::add_tag(
			std::string const& name,
			std::string const& type,
			std::string const& value
		) {
			assert( type.size() == 1 ) ;
			using genfile::string_utils::to_repr ;
			VariantEntry v ;
			switch( type[0] ) {
				case 'i':
					v = to_repr< VariantEntry::Integer >( value ) ;
					break ;
				case 'f':
					v = to_repr< double >( value ) ;
					break ;
				case 'A':
				case 'Z':
				case 'J':
				case 'H':
				case 'B':
					v = value ;
					break ;
				default:
					throw InputError(
						"genfile::gfa::TaggedElt::add_tag()"
						"Unrecognised type '" + type + '"'
					) ;
			} ;
			m_tags[name] = v ;
		}

		Segment::Segment(
			std::size_t index,
			std::string const& name,
			boost::optional< std::string > const& sequence
		):
			TaggedElt( index ),
			m_name( name ),
			m_sequence( sequence )
		{}

		Link::Link(
			std::size_t index,
			std::size_t const& from,
			Orientation const& from_orient,
			std::size_t const& to,
			Orientation const& to_orient,
			boost::optional< std::string > const& overlap_cigar
		):
			TaggedElt( index ),
			m_from( from ),
			m_from_orient( from_orient ),
			m_to( to ),
			m_to_orient( to_orient ),
			m_overlap_cigar( overlap_cigar )
		{}

		ALine::ALine(
			std::size_t index,
			std::size_t contig_id,
			uint32_t contig_start,
			Orientation strand,
			std::string read_name,
			uint32_t read_start,
			uint32_t read_end,
			int read_id,
			HaplotypeStatus status
		):
			TaggedElt( index ),
			m_contig_id( contig_id ),
			m_contig_start( contig_start ),
			m_strand( strand ),
			m_read_name( read_name ),
			m_read_start( read_start ),
			m_read_end( read_end ),
			m_read_id( read_id ),
			m_status( status )
		{}

		GFA::UniquePtr GFA::create( std::istream& input  ) {
			return UniquePtr( new GFA( input )) ;
		}

		GFA::GFA( std::istream& input ) {
			std::vector< std::string > L_lines ;
			std::vector< std::string > A_lines ;

			UniquePtr result ;

			std::string line ;
			using genfile::string_utils::slice ;
			using genfile::string_utils::to_repr ;
			while( std::getline( input, line )) {
				if( line.size() == 0 ) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"Empty line found in input."
					) ;
				}
				std::vector< slice > elts = slice( line ).split( "\t" ) ;
				switch( line[0] ) {
					case '#':
						break ;
					case 'S':
						if( elts.size() < 3 ) {
							throw genfile::InputError(
								"genfile::gfa::GFA::GFA()",
								"L line is malformed: \"" + line + "\"."
							) ;
						}
						// segment
						m_segments.push_back(
							Segment(
								m_segments.size(),
								elts[1],
								std::string( elts[2] )
							)
						) ;
						m_segment_indices[elts[1]] = m_segments.size()-1 ;
						if( elts.size() > 3 ) {
							for( auto elt: elts ) {
								auto components = elt.split( ":" ) ;
								m_segments.back().add_tag(
									components[0],
									components[1],
									components[2]
								) ;
							}
						}
						break ;
					case 'L':
						// link
						// We process links later to ensure that
						// all segments have been processed first.
						L_lines.push_back( line ) ;
						break ;
					case 'A':
						// A line
						// We process A lines later to ensure that
						// all segments have been processed first.
						A_lines.push_back( line ) ;
						break ;
					default:
						throw genfile::InputError(
							"genfile::gfa::GFA::create()",
							"Unrecognised line type '" + line.substr(0,1) + "'"
						) ;
				}
			}

			for( auto line: L_lines ) {
				std::vector< slice > elts = slice( line ).split( "\t" ) ;
				if( elts.size() < 6 ) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"L line is malformed (less than 6 entries): \"" + line + "\"."
					) ;
				}
				if( !(elts[2].size() == 1 && ( elts[2] == "+" || elts[2] == "-" ) )) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"L line is malformed (orientation is not '+' or '-')."
					) ;
				}
				auto where1 = m_segment_indices.find( elts[1] ) ;
				auto where2 = m_segment_indices.find( elts[3] ) ;
				if( where1 == m_segment_indices.end() || where2 == m_segment_indices.end() ) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"L line is malformed (segment \"" + elts[1] + "\" or \"" + elts[3] + "\" was not in the file."
					) ;
				}
				m_links.push_back(
					Link(
						m_links.size(),
						where1->second,
						parse_orientation(elts[2]),
						where2->second,
						parse_orientation(elts[4]),
						std::string(elts[5])
					)
				) ;
				m_links_from[ where1->second ].push_back( m_links.size() - 1 ) ;
				m_links_to  [ where2->second ].push_back( m_links.size() - 1 ) ;
			}

			for( auto line: A_lines ) {
				std::vector< slice > elts = slice( line ).split( "\t" ) ;
				if( elts.size() < 9 ) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"A line is malformed (less than 9 entries): \"" + line + "\"."
					) ;
				}
				auto where1 = m_segment_indices.find( elts[1] ) ;
				if( where1 == m_segment_indices.end() ) {
					throw genfile::InputError(
						"genfile::gfa::GFA::create()",
						"A line is malformed (segment \"" + elts[1] + "\" was not in the file."
					) ;
				}
				m_alines.push_back(
					ALine(
						m_alines.size(),
						where1->second,
						to_repr< uint32_t >( elts[2] ),
						parse_orientation(elts[3]),
						elts[4],
						to_repr< uint32_t >( elts[5] ),
						to_repr< uint32_t >( elts[6] ),
						to_repr< uint32_t >( elts[7].substr( 5, elts[7].size() )),
						parse_haplotype_status(elts[8])
					)
				) ;
				m_alines_for[where1->second].push_back( m_alines.size() - 1 ) ;
			}
		}
	}
}

