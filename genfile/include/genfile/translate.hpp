//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GENFILE_TRANSLATE_HPP
#define GENFILE_TRANSLATE_HPP

#include <string>

namespace genfile {
	std::string translate(
		std::string const& sequence,
		bool truncate_at_stops
	) ;
}

#endif
