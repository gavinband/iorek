# find-homopolymers

`find-homopolymers` is a tool to find all homopolymer tracts - and also di- and tri-nucleotide repeat tracts - in a FASTA file.

It:

* reads in a FASTA file
* walks the sequences to find homopolymer and short repeat tracts
* and outputs a file listing their location and length.

One use-case is to classify read errors by their location in homopolymer tracts - see
[`tabulate-mismatches`](tabulate-mismatches.md) for a way to do this.

## Synopsis

Get program options:

```
$ find-homopolymers -help

Welcome to find-homopolymers
(version: 0.6, revision 0aca522)

(C) 2009-2022 University of Oxford

Usage: find-homopolymers <options>

OPTIONS:
Input / output file options:
  -fasta <a> <b>...: Path of FASTA file(s) to operate on.
        -max-length: max length of nucleotide repeat to look for.  Must be <= 3.  Defaults to "3".
             -o <a>: Path of output file.  Defaults to "-".
```

## Example

An example can be found in `example/mismatches`:
```
$ cat example/mismatches/example.fa
>contig1
ATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCG
>contig2
ACTGAACTGAAACTGAAAACTGACACTGAGACACTGACGACTGACTGGACACTACT
```

Note in the above that `contig1` has no repeats <= 3 bases (it is all `ATCG`), while `contig2` has several.  Let's find them:

```
% find-homopolymers -fasta example/mismatches/example.fa

Welcome to find-homopolymers
(version: 0.6, revision 0aca522)

(C) 2009-2022 University of Oxford

Loading "example/mismatches/example.fa"                     : [******************************] (1/1,0.0s,40.1/s)
Processing "contig1"                                        : [******************************] (64/64,0.0s,301886.8/s)
Processing "contig2"                                        : [******************************] (56/56,0.0s,266666.7/s)
# Computed by find-homopolymers 2022-05-23 11:39:45
# Coordinates are 1-based, closed.
sequence	start	end	repeat	length
contig2	5	6	A	2
contig2	10	12	A	3
contig2	16	19	A	4
contig2	23	26	AC	4
contig2	28	31	GA	4
contig2	31	34	AC	4
contig2	36	41	GAC	6
contig2	47	48	G	2
contig2	49	52	AC	4
contig2	51	56	ACT	6
```
