# translatorator

`translatorator` is a tool to locate, and call, specific subsequences of assembly-level DNA sequences or PacBio HiFi reads.
Its particular USP is that it looks up sub-sequences based on pairs of DNA kmers at the 5' and 3' end.

A key use case is extracting the coding sequence of a gene based on kmers identifying the start and end of each exon.

**Note** `translatorator` is under development and its feature set, design, and usage miught change.

## Features:

* Can work with FASTA input or with aligned BAM/SAM/CRAM files
* Finds sub-sequences of input sequences based on the presence of specific kmer pairs in the specific order given
* In `asm` mode, outputs one sequence per input sequence (provided the kmers match).
* In `hifi` mode, allows for read errors and so outputs a representation of the sequences that are in the sample and their frequency amongst the reads.
* Examines sequences in both forward and reverse orientation
* Allows for IUPAC codes in specified kmers, to allow for known genetic variants
* Outputs matching sequences and their translation into amino acid sequence


## Running translatorator

`translatorator` has two modes.  If `-mode asm` is specified, `translatorator` assumes that each input sequence (or read) is a fully correct / assembled sequence and proceeds accordingly.  The file specified by `-output-sequences` will then detail the sub-sequences of each input sequence (or read) that were matched by the kmers.


## Basic example 1:

Given an input sequence like this in `mysequence.fasta`:

```
>my_sequence
ACGTGGTCACCGTCGTCGTAAGTCATAC
```

and kmer pairs `CGTG` - `GTCG` and `GTAA` - `CATA`, 

```sh
translatorator -sequences mysequence.fasta -cds-kmers CGTG GTCG GTAA CATA -output-sequences output.tsv
```
will produce

translatorator will figure out that the input sequence segments like this:

```
aCGTGGTCACCGTCGtcGTAAGTCATAc
```
and will output a file reflecting this:
```t
# Written by translatorator
# Kmer pairs are:
# 1: CGTG / GTCG
# 2: GTAA / CATA
file    read_id strand  start_1 end_1   start_2 end_2   dna_sequence    aa_sequence
mysequence      my_sequence     +       2       15      18      27      CGTGGTCACCGTCGGTAAGTCATA        RGHRRXVI
```

## Kmer matching notes

`translatorator` only considers an input sequence to match a set of kmers `(k1,k2), (k3,k4), ...` if

* there's a unique way to match k1, k2, ... to the sequence (allowing for IUPAC codes) in the given order; and
* the matching locations of k1, k2, ... don't overlap.

