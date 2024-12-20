import os.path
import glob

VERSION = '0.9'

subdirs = [ '3rd_party', 'appcontext', 'apps', 'statfile', 'genfile', 'qcdb', 'lib' ]

def options( opt ):
		opt.load( 'compiler_cxx' )
		opt.load( 'compiler_c' )

def configure( cfg ):
	cfg.load( 'compiler_c')
	cfg.load( 'compiler_cxx')
	flags = {
		'CXXFLAGS': cfg.env[ 'CXXFLAGS' ],
		'CFLAGS': cfg.env[ 'CFLAGS' ],
		'LINKFLAGS': cfg.env[ 'LINKFLAGS' ]
	}
	flags['CXXFLAGS'].extend( [
		'-std=c++11',
		'-Wall',
		'-pedantic',
		'-Wno-unused-local-typedefs', '-Wno-c++11-long-long', '-Wno-deprecated-declarations', '-Wno-long-long',
		'-Wno-c11-extensions',
		'-Wno-int-in-bool-context',
		'-fPIC'
	] )
	flags['CFLAGS'].extend( [
		'-std=c99',
		'-Wall',
		'-pedantic',
		'-Wno-unused-local-typedefs', '-Wno-c++11-long-long', 'Wno-deprecated-declarations', '-Wno-long-long',
		'-Wno-c11-extensions',
		'-Wno-int-in-bool-context',
		 '-fPIC'
	] )
	mode = 'debug'
	if mode == 'release':
			flags['CXXFLAGS'] += [ '-O3' ]
			flags['CFLAGS'] = [ '-O3' ]
	elif mode == 'debug':
			flags['CXXFLAGS'] += [ '-g' ]
			flags['CFLAGS'] = [ '-g' ]
	else:
			raise Exception( "Unknown value for mode, please specify debug or release" )

	cfg.env['CXXFLAGS'] = flags['CXXFLAGS']
	cfg.env['CFLAGS'] = flags['CFLAGS']
	cfg.env['LINKFLAGS'] = flags['LINKFLAGS']

	cfg.check_cxx( lib='z', uselib_store='zlib', msg = 'zlib' )
	import platform
	if platform.system() == "Darwin":
		pass
	else:
		cfg.check_cxx( lib='rt', uselib_store='rt', msg = 'rt' )
		cfg.check_cxx( lib='pthread', uselib_store='pthread', msg = 'pthread' )
		cfg.check_cxx( lib='dl', uselib_store='dl', msg = 'dl' )
	
	cfg.recurse( subdirs )
	cfg.write_config_header( 'config/config.hpp' )
	

def compute_revision(task):
	import os, sqlite3
	revision = "unknown"
	filename = ".fslckout"
	if os.path.exists( filename ):
		db = sqlite3.connect( filename )
		c = db.cursor()
		c.execute( "SELECT value FROM vvar WHERE name == 'checkout-hash'" )
		result = c.fetchone()
		revision = result[0][0:10]
	elif os.path.exists( '.git' ):
		import subprocess
		revision = subprocess.check_output( ['git', 'rev-parse', '--short', 'HEAD'] ).decode('ascii').strip()
	else:
		print( "Neither .fslckout nor .git exist.\n" )
	target = open( task.outputs[0].abspath(), "w" )
	target.write(
	"""
#ifndef PACKAGE_REVISION_HPP
#define PACKAGE_REVISION_HPP
namespace globals {
char const* const package_version = "%s" ;
char const* const package_revision = "%s" ;
}
#endif
""" % ( VERSION, revision ) )
	target.close()
	return 0 # success

def build( bld ):
	bld(
		target = 'svelte_config',
		includes = "./",
		export_includes = "./"
	)
	bld(
		name = 'package_revision',
		target = "package_revision_autogenerated.hpp",
		always = True,
		on_result = True,
		rule = compute_revision,
		ext_out = [ "package_revision_autogenerated.hpp" ],
	)
	bld.add_group()
	bld.recurse( subdirs )

