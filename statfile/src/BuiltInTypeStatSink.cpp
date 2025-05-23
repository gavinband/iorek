
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unistd.h>
#include <memory>
#include <string>
#include "statfile/StatSink.hpp"
#include "statfile/DelimitedStatSink.hpp"

namespace statfile {
	BuiltInTypeStatSink::UniquePtr BuiltInTypeStatSink::open( std::string const& filename ) {
		BuiltInTypeStatSink::UniquePtr result ;
		FileFormatType format = get_file_format_type_indicated_by_filename( filename ) ;
		
		if( format == statfile::e_CommaDelimitedFormat ) {
			result.reset( new statfile::DelimitedStatSink( filename, "," ) ) ;
		}
		else if( format == statfile::e_TabDelimitedFormat ) {
			result.reset( new statfile::DelimitedStatSink( filename, "\t" ) ) ;
		}
		else if( format == statfile::e_SpaceDelimited ) {
			result.reset( new statfile::DelimitedStatSink( filename, " " ) ) ;
		}
		else {
			// default to tab-separated format.
			result.reset( new statfile::DelimitedStatSink( filename, "\t" ) ) ;
		}
		return result ;
	}
	
	BuiltInTypeStatSink::UniquePtr NullBuiltInTypeStatSink::open() {
		return BuiltInTypeStatSink::UniquePtr( new NullBuiltInTypeStatSink ) ;
	}
}

