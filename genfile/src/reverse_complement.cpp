
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include "genfile/reverse_complement.hpp"

namespace genfile {
	std::string reverse_complement( std::string const& sequence ) {
		return reverse_complement( sequence.begin(), sequence.end() ) ;
	}
}
