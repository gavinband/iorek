ACLOCAL_AMFLAGS = -I m4
DISTCHECK_CONFIGURE_FLAGS=--enable-swig --enable-all-binding

EXTRA_DIST = doc/jellyfish.pdf doc/jellyfish.man README LICENSE-BSD-3-Clause LICENSE-GPL-3.0 # jellyfish.spec
man1_MANS = doc/jellyfish.man

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jellyfish-2.0.pc

AM_LDFLAGS = -pthread $(HTSLIB_LIBS) $(HTSLIB_RPATH) # $(VALGRIND_LIBS)
AM_CPPFLAGS = -Wall -Wnon-virtual-dtor -I$(srcdir) -I$(srcdir)/include $(VALGRIND_CFLAGS)
AM_CXXFLAGS = $(ALL_CXXFLAGS) $(HTSLIB_CFLAGS)
#AM_LDADD    = $(HTSLIB_LIBS) -lpthread

noinst_HEADERS = $(YAGGO_SOURCES)
bin_PROGRAMS =
dist_bin_SCRIPTS =
data_DATA =
BUILT_SOURCES = $(YAGGO_SOURCES)
CLEANFILES =
DISTCLEANFILES = $(BUILT_SOURCES)

# Yaggo automatic rules with silencing
V_YAGGO = $(V_YAGGO_$(V))
V_YAGGO_ = $(V_YAGGO_$(AM_DEFAULT_VERBOSITY))
V_YAGGO_0 = @echo "  YAGGO   " $@;
.yaggo.hpp:
	$(V_YAGGO)$(YAGGO) --license $(srcdir)/header-license -o $@ $<

YAGGO_SOURCES = # Append all file to be built by yaggo

# What to build
bin_PROGRAMS += bin/jellyfish
lib_LTLIBRARIES = libjellyfish-2.0.la
LDADD = libjellyfish-2.0.la # $(VALGRIND_LIBS)
check_PROGRAMS = bin/generate_sequence bin/fastq2sam

############################
# Build Jellyfish the exec #
############################
bin_jellyfish_SOURCES = sub_commands/jellyfish.cc	\
                        sub_commands/count_main.cc	\
                        sub_commands/info_main.cc	\
                        sub_commands/dump_main.cc	\
                        sub_commands/histo_main.cc	\
                        sub_commands/stats_main.cc	\
                        sub_commands/merge_main.cc	\
                        sub_commands/bc_main.cc		\
                        sub_commands/query_main.cc	\
                        sub_commands/cite_main.cc	\
                        sub_commands/mem_main.cc	\
                        jellyfish/merge_files.cc
bin_jellyfish_LDFLAGS = $(AM_LDFLAGS) $(STATIC_FLAGS)

YAGGO_SOURCES += sub_commands/count_main_cmdline.hpp	\
                 sub_commands/info_main_cmdline.hpp	\
                 sub_commands/dump_main_cmdline.hpp	\
                 sub_commands/histo_main_cmdline.hpp	\
                 sub_commands/stats_main_cmdline.hpp	\
                 sub_commands/merge_main_cmdline.hpp	\
                 sub_commands/bc_main_cmdline.hpp	\
                 sub_commands/query_main_cmdline.hpp	\
                 sub_commands/cite_main_cmdline.hpp	\
                 sub_commands/mem_main_cmdline.hpp

######################################
# Build Jellyfish the shared library #
######################################
libjellyfish_2_0_la_LDFLAGS = -version-info 2:0:0
libjellyfish_2_0_la_SOURCES = lib/rectangular_binary_matrix.cc		\
                              lib/mer_dna.cc lib/storage.cc		\
                              lib/allocators_mmap.cc lib/misc.cc	\
                              lib/int128.cc lib/thread_exec.cc		\
                              lib/jsoncpp.cpp lib/time.cc	\
                              lib/generator_manager.cc


library_includedir=$(includedir)/jellyfish-@PACKAGE_VERSION@/jellyfish
JFI = include/jellyfish
library_include_HEADERS = $(JFI)/allocators_mmap.hpp			\
                          $(JFI)/backtrace.hpp $(JFI)/atomic_gcc.hpp	\
                          $(JFI)/large_hash_array.hpp $(JFI)/err.hpp	\
                          $(JFI)/misc.hpp				\
                          $(JFI)/offsets_key_value.hpp			\
                          $(JFI)/int128.hpp				\
                          $(JFI)/rectangular_binary_matrix.hpp		\
                          $(JFI)/mer_dna.hpp $(JFI)/storage.hpp		\
                          $(JFI)/simple_circular_buffer.hpp		\
                          $(JFI)/circular_buffer.hpp			\
                          $(JFI)/atomic_field.hpp			\
                          $(JFI)/compare_and_swap.hpp			\
                          $(JFI)/divisor.hpp				\
                          $(JFI)/large_hash_iterator.hpp		\
                          $(JFI)/jellyfish.hpp $(JFI)/thread_exec.hpp	\
                          $(JFI)/stream_iterator.hpp			\
                          $(JFI)/mer_overlap_sequence_parser.hpp	\
                          $(JFI)/whole_sequence_parser.hpp		\
                          $(JFI)/parser_common.hpp			\
                          $(JFI)/sam_format.hpp				\
                          $(JFI)/binary_dumper.hpp			\
                          $(JFI)/sorted_dumper.hpp			\
                          $(JFI)/text_dumper.hpp $(JFI)/dumper.hpp	\
                          $(JFI)/time.hpp $(JFI)/mer_heap.hpp		\
                          $(JFI)/token_ring.hpp				\
                          $(JFI)/locks_pthread.hpp			\
                          $(JFI)/file_header.hpp			\
                          $(JFI)/generic_file_header.hpp		\
                          $(JFI)/json.h $(JFI)/hash_counter.hpp		\
                          $(JFI)/mapped_file.hpp			\
                          $(JFI)/mer_dna_bloom_counter.hpp		\
                          $(JFI)/bloom_common.hpp			\
                          $(JFI)/bloom_counter2.hpp			\
                          $(JFI)/bloom_filter.hpp			\
                          $(JFI)/cooperative_pool.hpp			\
                          $(JFI)/cooperative_pool2.hpp			\
                          $(JFI)/stream_manager.hpp			\
                          $(JFI)/generator_manager.hpp			\
                          $(JFI)/cpp_array.hpp				\
                          $(JFI)/mer_iterator.hpp			\
                          $(JFI)/atomic_bits_array.hpp			\
                          $(JFI)/stdio_filebuf.hpp			\
                          $(JFI)/mer_qual_iterator.hpp


noinst_HEADERS += jellyfish/fstream_default.hpp jellyfish/dbg.hpp	\
                  jellyfish/randomc.h jellyfish/merge_files.hpp

###############
# Build tests #
###############
bin_generate_sequence_SOURCES = jellyfish/generate_sequence.cc		\
                                jellyfish/mersenne.cpp			\
                                jellyfish/backtrace.cc			\
                                jellyfish/dbg.cc
YAGGO_SOURCES += jellyfish/generate_sequence_cmdline.hpp

bin_fastq2sam_SOURCES = jellyfish/fastq2sam.cc
YAGGO_SOURCES += jellyfish/fastq2sam_cmdline.hpp

#########
# Tests #
#########
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)
AM_SH_LOG_FLAGS =

TESTS = tests/generate_sequence.sh tests/parallel_hashing.sh	\
        tests/merge.sh tests/bloom_filter.sh tests/big.sh	\
        tests/subset_hashing.sh tests/multi_file.sh		\
        tests/bloom_counter.sh tests/large_key.sh tests/sam.sh	\
        tests/small_mers.sh

EXTRA_DIST += $(TESTS)
clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-cd tests-data && rm -f *

tests/parallel_hashing.log: tests/generate_sequence.log
tests/subset_hashing.log: tests/generate_sequence.log
tests/bloom_filter.log: tests/generate_sequence.log
tests/bloom_counter.log: tests/generate_sequence.log
tests/multi_file.log: tests/generate_sequence.log
tests/merge.log: tests/generate_sequence.log
tests/min_qual.log: tests/generate_fastq_sequence.log
tests/large_key.log: tests/generate_sequence.log
tests/quality_filter.log: tests/generate_sequence.log
tests/sam.log: tests/generate_sequence.log
tests/small_mers.log: tests/generate_sequence.log

# SWIG tests
TESTS += tests/swig_python.sh tests/swig_ruby.sh tests/swig_perl.sh
tests/swig_python.log: tests/generate_sequence.log
tests/swig_ruby.log: tests/generate_sequence.log
tests/swig_perl.log: tests/generate_sequence.log
EXTRA_DIST += swig/python/test_mer_file.py swig/python/test_hash_counter.py swig/python/test_string_mers.py
EXTRA_DIST += swig/ruby/test_mer_file.rb swig/ruby/test_hash_counter.rb swig/ruby/test_string_mers.rb
EXTRA_DIST += swig/perl5/t/test_mer_file.t swig/perl5/t/test_hash_counter.t swig/perl5/t/test_string_mers.t


##############
# Unit tests #
##############
TESTS += unit_tests/unit_tests.sh
check_PROGRAMS += bin/test_all

bin_test_all_SOURCES = unit_tests/test_main.cc				\
	               unit_tests/test_misc.cc				\
	               unit_tests/test_offsets_key_value.cc		\
	               unit_tests/test_simple_circular_buffer.cc	\
	               unit_tests/test_rectangular_binary_matrix.cc	\
	               unit_tests/test_mer_dna.cc			\
	               unit_tests/test_large_hash_array.cc		\
	               unit_tests/test_mer_overlap_sequence_parser.cc	\
	               unit_tests/test_file_header.cc			\
	               unit_tests/test_mer_iterator.cc			\
	               unit_tests/test_hash_counter.cc			\
	               unit_tests/test_mer_heap.cc			\
	               unit_tests/test_stream_iterator.cc		\
	               unit_tests/test_token_ring.cc			\
	               unit_tests/test_text_dumper.cc			\
	               unit_tests/test_dumpers.cc			\
	               unit_tests/test_mapped_file.cc			\
	               unit_tests/test_int128.cc			\
	               unit_tests/test_mer_dna_bloom_counter.cc		\
	               unit_tests/test_whole_sequence_parser.cc		\
	               unit_tests/test_allocators_mmap.cc		\
	               unit_tests/test_cooperative_pool2.cc		\
	               unit_tests/test_generator_manager.cc		\
	               unit_tests/test_atomic_bits_array.cc		\
	               unit_tests/test_stdio_filebuf.cc
bin_test_all_SOURCES += jellyfish/backtrace.cc

bin_test_all_CPPFLAGS = $(AM_CPPFLAGS) -DJSON_IS_AMALGAMATION=1
bin_test_all_CXXFLAGS = $(AM_CXXFLAGS) -I$(srcdir)/unit_tests/gtest/include -I$(srcdir)/unit_tests -I$(srcdir)/include -Wno-unused-variable
bin_test_all_LDADD = libgtest.la $(LDADD)
YAGGO_SOURCES += unit_tests/test_main_cmdline.hpp
noinst_HEADERS += unit_tests/test_main.hpp

#################
# SWIG bindings #
#################
include swig/Makefile.am

include gtest.mk
-include $(srcdir)/development.mk

# Print the value of a variable
print-%:
	@echo -n $($*)
