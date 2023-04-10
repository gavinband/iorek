
//					Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//		(See accompanying file LICENSE_1_0.txt or copy at
//					http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_ANNOTATION_GFFRECORD
#define GENFILE_ANNOTATION_GFFRECORD 1

#include <string>
#include <iostream>
#include <functional>
#include "genfile/VariantEntry.hpp"
#include "genfile/string_utils/string_utils.hpp"
#include "genfile/string_utils/slice.hpp"

namespace genfile {
	namespace annotation {
		struct GFFRecord: public boost::noncopyable {
		public:
			typedef genfile::string_utils::slice slice ;
		private:
			static genfile::VariantEntry parse_attributes( std::string const& attributes, std::string const& attribute ) ;

		public:
			static GFFRecord parse( std::string const& line ) ;
			GFFRecord( GFFRecord const& other ) ;
			
			slice const& sequence() const { return m_elts[0] ; }
			slice const& source() const { return m_elts[1] ; }
			slice const& feature() const { return m_elts[2] ; }
			genfile::Position start() const { return m_start ; }
			genfile::Position end() const { return m_end ; }
			slice const& score() const { return m_elts[5] ; }
			slice const& strand() const { return m_elts[6] ; }
			slice const& phase() const { return m_elts[7] ; }
			slice const& attributes() const { return m_elts[8] ; }
			genfile::VariantEntry const& ID() const { return m_ID ; }
			genfile::VariantEntry const& parent() const { return m_parent ; }
			genfile::VariantEntry const& gene_name() const { return m_gene_name ; }

		private:
			std::string m_data ;
			std::vector< slice > m_elts ;
			genfile::Position m_start ;
			genfile::Position m_end ;
			genfile::VariantEntry m_ID ;
			genfile::VariantEntry m_parent ;
			genfile::VariantEntry m_description ;
			genfile::VariantEntry m_gene_name ;

			GFFRecord() ;
		} ;

		void parse_gff(
			std::istream& input,
			std::function< void( GFFRecord const& record ) > callback
		) ;
	}
}

#endif
