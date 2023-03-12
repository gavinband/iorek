
//          Copyright Gavin Band 2008 - 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef APPCONTEXT_OPTIONPROCESSOR_HPP
#define APPCONTEXT_OPTIONPROCESSOR_HPP


#include <string>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>
#include <functional>
#include "appcontext/OptionDefinition.hpp"

namespace appcontext {
	struct OptionProcessingException: public std::exception
	{
		OptionProcessingException( std::string option, std::vector< std::string > values, std::string msg ) ;
		OptionProcessingException( std::string option, std::string msg ) ;
		~OptionProcessingException() throw() ;
	
		char const* what() const throw() { return "OptionProcessingException" ; }

		std::string const& option() const { return m_option ; }
		std::vector< std::string > const& values() const { return m_values ; }
		std::string const& message() const { return m_msg ; }
	private:
	
		std::string m_option ;
		std::vector< std::string > m_values ;
		std::string m_msg ;
	} ;

	struct OptionParseException: public OptionProcessingException
	{
		OptionParseException( std::string option, std::vector< std::string > values, std::string msg ) ;
	} ;

	struct OptionValueInvalidException: public OptionProcessingException
	{
		OptionValueInvalidException( std::string option, std::vector< std::string > values, std::string msg ) ;
	} ;

	struct OptionProcessorHelpRequestedException: public std::exception {
		char const* what() const throw() { return "OptionProcessorHelpRequestedException" ; }
	} ;

	struct OptionProcessorMutuallyExclusiveOptionsSuppliedException: public std::exception {
		OptionProcessorMutuallyExclusiveOptionsSuppliedException( std::string const& option1, std::string const& option2 )
		: m_option1( option1 ),
		m_option2( option2 )
		{}
	
		~OptionProcessorMutuallyExclusiveOptionsSuppliedException() throw() {}

		char const* what() const throw() { return "OptionProcessorMutuallyExclusiveOptionsSuppliedException" ; }

		std::string const& first_option() const { return m_option1 ; }
		std::string const& second_option() const { return m_option2 ; }

	private:
	
		std::string m_option1 ;
		std::string m_option2 ;
	} ;

	struct OptionProcessorImpliedOptionNotSuppliedException: public std::exception {
		OptionProcessorImpliedOptionNotSuppliedException( std::string const& option1, std::string const& option2 )
		: m_option1( option1 ),
		m_option2( option2 )
		{}
	
		~OptionProcessorImpliedOptionNotSuppliedException() throw() {}

		char const* what() const throw() { return "OptionProcessorImpliedOptionNotSuppliedException" ; }

		std::string const& first_option() const { return m_option1 ; }
		std::string const& second_option() const { return m_option2 ; }

	private:
	
		std::string m_option1 ;
		std::string m_option2 ;
	} ;

	class OptionProcessor {
		public:
			typedef std::unique_ptr< OptionProcessor > UniquePtr ;
			typedef std::map< std::string, OptionDefinition > OptionDefinitions ;
			typedef std::map< std::string, std::vector< std::string > > OptionValues ; 
			typedef std::map< std::string, std::pair< std::vector< std::string >, std::string > > OptionValueMap ;

		public:
			OptionProcessor() ;
			OptionProcessor( OptionProcessor const& other ) ;
			virtual ~OptionProcessor() ;
		
			// Return the current option definitions
			OptionDefinitions const& get_option_definitions() const ;
			// Operator [] provides the main way to create new option definitions.
			// It has specific semantics: can only be called once (to create a new option)
			// and returns an OptionDefinition object whose methods can be used to set properties
			// of the option. 
			OptionDefinition& operator[]( std::string const& arg ) ;
			// Return the current option definition for the given option (which must be already defined.)
			OptionDefinition const& operator[] ( std::string const& arg ) const ;

			// Declare an current option 'group'.
			// All options defined henceforth are treated as in this option group, until
			// another group is declared.
			void declare_group( std::string const& ) ;

			// Declare the name of the 'help' option.
			void set_help_option( std::string const& help_option_name ) { m_help_option_name = help_option_name ; }

			// Set logic for mutual exclusion / implicatio between options and groups
			void option_excludes_option( std::string const& excluding_option, std::string const& excluded_option ) ;
			void option_excludes_group( std::string const& excluding_option, std::string const& excluded_option_group ) ;
			void option_implies_option( std::string const& option, std::string const& implied_option ) ;

			// Parse the defined options from argc/argv.
			// If any errors arise, an OptionProcessingException will be raised.
			virtual void process( int argc, char** argv ) ;

			// check if an option has been defined.
			bool check_if_option_is_defined( std::string const& arg ) const ;
			// check if the given option (which must be valid) has a value supplied by the user.
			bool check_if_option_was_supplied( std::string const& arg ) const ;
			bool check( std::string const& arg ) const { return check_if_option_was_supplied( arg ) ; }
			// check if the given option (which must be valid) has a value (either supplied or default).
			bool check_if_option_has_value( std::string const& arg ) const ;
			bool has_value( std::string const& arg ) const { return check_if_option_has_value( arg ) ; }
			// check if any option in the given group was supplied.
			bool check_if_option_was_supplied_in_group( std::string const& group ) const ;

			// get the value of an option (which must be valid) as a string
			// The option must have exactly one value.
			std::string get_value( std::string const& arg ) const ;
			// get the values of an option (which must be valid) as a vector of strings
			std::vector< std::string > get_values( std::string const& arg ) const ;

			// get the value of the given option, coerced to the given type.
			template< typename T >
			T get_value( std::string const& arg ) const {
				std::istringstream s( get_value( arg )) ;
				T t ;
				s >> t ;
				s.peek() ;
				if( !s.eof() ) {
					throw OptionValueInvalidException( arg, get_values( arg ), "Type error." ) ;
				}
				return t ;
			}

			// synonym for the above
			template< typename T >
			T get( std::string const& arg ) const {
				return get_value< T >( arg ) ;
			}

			// get all values of the given option, as a vector of the given type.
			template< typename T >
			std::vector< T > get_values( std::string const& arg ) const {
				std::vector< std::string > values = get_values( arg ) ;
				std::vector<T> result ;
				result.reserve( values.size() ) ;
				for( std::size_t i = 0; i < values.size(); ++i ) {
					std::istringstream istr( values[i] ) ;
					T t ;
					istr >> t ;
					result.push_back( t ) ;
				}
				return result ;
			}

			// Output a human-readable version of these options to the stream.
			friend std::ostream& operator<<( std::ostream& aStream, OptionProcessor::OptionDefinitions const& option_definitions ) ;
			friend std::ostream& operator<<( std::ostream& aStream, OptionProcessor const& options ) ;

			// Get the 'help' option name.
			std::string const& get_help_option_name() const { return m_help_option_name ; }

			// Return a structure reflecting the current set of options and values.
			enum { eUserSupplied = 0x1, eDefaulted = 0x2, eNotSet = 0x4 } ;
			OptionValueMap get_values_as_map(
				int const value_types = eUserSupplied
			) const ;

		public:
			// checks
			typedef std::function< void ( OptionProcessor& ) > Check ;
			void add_check( Check check ) ;
			void check_equal_multiplicity( std::string const& option1, std::string const& option2 ) ;

		private:
			void calculate_option_groups() ;
			// Parse the options.  Store option values.  Ignore, but store unknown args for later reference
			void parse_options( int argc, char** argv ) ;
			bool try_to_parse_named_option_and_values( int argc, char** argv, int& i ) ;
			bool try_to_parse_positional_option( int argc, char** argv, int& i ) ;
			void process_unknown_options() ;
			void check_required_options_are_supplied() const ;
			void check_mutually_exclusive_options_are_not_supplied() const ;
			void check_implied_options_are_supplied() const ;
			virtual void check_option_values() ;
			void run_user_checks() ;
			std::string get_default_value( std::string const& arg ) const ;
			std::vector< std::string > get_default_values( std::string const& arg ) const ;
			std::size_t get_maximum_option_length() const ;
			std::size_t get_maximum_option_length( std::string const& group ) const ;
			std::string format_option_and_arguments( std::string const& option_name ) const ;
			void format_options( std::ostream& ) const ;
			void format_option_group( std::ostream&, std::string const& ) const ;
			void format_option_and_description( std::ostream& aStream, std::string const& option_name, std::size_t max_option_length ) const ;
		
			OptionDefinitions m_option_definitions ;
			OptionValues m_option_values ;
			std::map< int, std::string > m_unknown_options ;
			std::string m_current_group ;
			std::map< std::string, std::set< std::string > > m_option_groups ;
			std::vector< std::string > m_option_group_names ;
			std::string m_help_option_name ;
			std::map< std::string, std::set< std::string > > m_option_exclusions ;
			std::map< std::string, std::set< std::string > > m_option_implications ;
			std::vector< Check > m_checks ;
	} ;
	
	template<>
	std::string OptionProcessor::get_value( std::string const& arg ) const ;
	template<>
	std::vector< std::string > OptionProcessor::get_values( std::string const& arg ) const ;
	
}

#endif

