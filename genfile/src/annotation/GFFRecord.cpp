
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <functional>
#include "boost/regex.hpp"
#include "genfile/VariantEntry.hpp"
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/annotation/GFFRecord.hpp"

namespace genfile {
	namespace annotation {
		GFFRecord::GFFRecord() {} ;

		genfile::VariantEntry GFFRecord::parse_attributes( std::string const& attributes, std::string const& attribute ) {
			// inefficient to compile re each time - improve this.
			const boost::regex re( attribute + "=([^;]+)" ) ;
			boost::match_results<std::string::const_iterator> match ;
			bool success = boost::regex_search( attributes, match, re ) ;
			if( !success || !match[1].matched) {
				return genfile::MissingValue() ;
			} else {
				return std::string( match[1].first, match[1].second ) ;
			}
		}

		GFFRecord GFFRecord::parse( std::string const& line ) {
			using genfile::string_utils::split ;
			GFFRecord result ;
			result.m_data = line ;
			result.m_elts = slice(result.m_data).split( "\t" ) ;
			assert( result.m_elts.size() == 9 ) ;
			result.m_start = genfile::string_utils::to_repr< genfile::Position >( result.m_elts[3] ) ;
			result.m_end = genfile::string_utils::to_repr< genfile::Position >( result.m_elts[4] ) ;
			result.m_ID = GFFRecord::parse_attributes( result.m_elts[8], "ID" ) ;
			result.m_parent = GFFRecord::parse_attributes( result.m_elts[8], "Parent" ) ;
			result.m_description = GFFRecord::parse_attributes( result.m_elts[8], "description" ) ;
			result.m_gene_name = GFFRecord::parse_attributes( result.m_elts[8], "gene_name" ) ;
			return result ;
		}

		GFFRecord::GFFRecord( GFFRecord const& other ):
			m_data( other.m_data ),
			m_elts( other.m_elts ),
			m_start( other.m_start ),
			m_end( other.m_end ),
			m_ID( other.m_ID ),
			m_parent( other.m_parent ),
			m_description( other.m_description ),
			m_gene_name( other.m_gene_name )
		{
			// fix elts to refer to our own data
			for( std::size_t i = 0; i < m_elts.size(); ++i ) {
				m_elts[i] = slice( m_data, m_elts[i].get_start(), m_elts[i].get_end() ) ;
			}
		}

		void parse_gff( std::istream& input, std::function< void( GFFRecord const& record ) > callback ) {
			std::string line ;
			while( std::getline( input, line )) {
				if( line.size() > 0 && line[0] != '#' ) {
					callback( GFFRecord::parse( line ) ) ;
				}
			}
		}
	}
}
