
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_DB_FILL_SQL_HPP
#define GENFILE_DB_FILL_SQL_HPP

#include <cassert>
#include <string>
#include <sstream>

namespace genfile {
	namespace db {
	
		std::string fill_SQL( std::string SQL ) ;
		std::string fill_SQL( std::string SQL, std::string const& value1 ) ;
		std::string fill_SQL( std::string SQL, char const* value1 ) ;
		template< typename T1 >
		std::string fill_SQL( std::string SQL, T1 const& value1 ) {
			std::ostringstream ostr ;
			ostr << value1 ;
			std::size_t pos = SQL.find( "%s" ) ;
			assert( pos != std::string::npos ) ;
			SQL.replace( pos, 2, ostr.str() ) ;
			return SQL ;
		}

		template< typename T1, typename T2 >
		std::string fill_SQL( std::string SQL, T1 const& value1, T2 const& value2 ) {
			return fill_SQL( fill_SQL( SQL, value1 ), value2 ) ;
		}
	}
}

#endif

