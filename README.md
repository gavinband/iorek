# Iorek #

This repository contains a collection of programs for analysing sequence data.

- `find-motifs` finds (short) specified motifs in sequences from a fasta file.
- `coverotron` computes per-base or per-window sequence read coverage from a set of BAM or CRAM files.
- `seemsa` reads a multiple sequence alignment FASTA file, and writes an interactive HTML visualisation of it.
- `tabulate-alignments` reports the count of reads split by read base (or insertion/deletion) and mapping quality at each position in a given range or ranges.
- `find-homopolymers` finds homopolymer tracts (and di- and tri-nucleotide repeats) in sequences from a FASTA file.  See the [documentation](doc/trunk/doc/find-homopolymers.md)
- `tabulate-mismatches` walks all reads and reports all mismatches / insertions / deletions stratified by their type and flanking sequence.  See the [documentation](doc/trunk/doc/tabulate-mismatches.md)

## License ##

Iorek is released under the Boost software license.  See the LICENSE.txt file for details.

Iorek makes use of several other libraries that are included in the source repository
and released under their own respective licenses.  These include:

* boost
* htslib
* SeqLib
* parallel-hashmap
* jellyfish2
* Eigen
* sqlite3
* zstandard
* catch2

Please see the respective license files in subdirectories of 3rd_party/ for details.

## Compilation ##

iorek is built using the waf build tool, which is bundled with the code.  A basic compilation cycle is:

```
$ ./waf configure
$ ./waf
```

Executables will appear in `build/apps/`.

You can also optionally specify an installation prefix and ask `waf` to install the result:
```
$ ./waf configure --prefix=[installation path]
$ ./waf install
```

Executables will be copied to `[installation path]/bin/`.

## Contributors ##

Iorek was written by Gavin Band.
