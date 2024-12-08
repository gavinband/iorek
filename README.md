# Iorek #

This repository contains a collection of programs for analysing sequence data.

- `find-motifs` finds (short) specified motifs in sequences from a fasta file.
- `find-homopolymers` finds homopolymer tracts, as well as di- and tri-nucleotide repeats, in sequences from a FASTA file.  See the [documentation](doc/find-homopolymers.md)
- `coverotron` computes per-base or per-window sequence read coverage from a set of BAM or CRAM files.
- `tabulate-alignments` reports the count of reads split by read base (or insertion/deletion) and mapping quality at each position in a given range or ranges.
- `iorek-qc` quantifies the presence of 'true' and 'error' kmers in a set of sequence reads (from a FASTQ file), based on a database of true kmers in [jellyfish2](https://github.com/zippav/Jellyfish-2) format.
- `tabulate-mismatches` walks all reads and reports all mismatches / insertions / deletions stratified by their type and flanking sequence.  See the [documentation](doc/tabulate-mismatches.md)
- `assess-qualities` quantifies the error rate predicted by base qualities, against the observed error rate, in a set of read alignments.
- `zoomsa` reads a multiple sequence alignment FASTA file, and writes an interactive HTML visualisation of it.

## License ##

Iorek is released under the Boost software license.  See the LICENSE.txt file for details.

Iorek makes use of several other libraries that are included in the source repository
and released under their own respective licenses.  These include:

* [boost](https://www.boost.org)
* [htslib](https://www.htslib.org)
* [SeqLib](https://github.com/walaj/SeqLib)
* [parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap)
* [moodycamel concurrent queue](https://github.com/cameron314/concurrentqueue)
* [jellyfish2](https://github.com/zippav/Jellyfish-2)
* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [sqlite3](https://www.sqlite.org)
* [zstandard](http://facebook.github.io/zstd/)
* [catch2](https://github.com/catchorg/Catch2)
* [wfa2](https://github.com/smarco/WFA2-lib)

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

Iorek was written by [Gavin Band](https://github.com/gavinband) with contributions from [Tom Roberts](https://github.com/Tomrrr1).
