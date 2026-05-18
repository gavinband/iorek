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

## Diagram

With input conceptually like this:
```
    kmer pairs:    CGTG      GTCG  GTAA  CATA
read1 sequence  GCACGTGGT-ACCGTCGTCGTAAGTCATAC
read2 sequence  GCACGTGGTCACCGTCGTCGTAAGTCATA
read3 sequence   CACGTGGTCACCGTCGTCGTAAGTCATACGCC
read4 sequence   CACGTGGTCACCGTCGTCGTAAGTCATACGCCGGC
read5 sequence    ACGTGGTCACCGTCGTTGTAAG-CATA
read6 sequence    ACGTGGTCACCGTCGTTGTAAG-CATACGCCGG
```

You get output conceptually like this (`-mode asm`):
```
 read sequence
read1 CGTGGTACCGTCGGTAAGTCATA
read2 CGTGGTCACCGTCGGTAAGTCATA
read3 CGTGGTCACCGTCGGTAAGTCATA
read4 CGTGGTCACCGTCGGTAAGTCATA
read5 CGTGGTCACCGTCGGTAAGCATA
read6 CGTGGTCACCGTCGGTAAGCATA
```

Or like this (`-mode hifi`):
```
cluster number_of_reads total_informative_reads exact_reads proportion sequence
      0               4                       6           3 0.666      CGTGGTCACCGTCGGTAAGTCATA
      1               2                       6           2 0.333      CGTGGTCACCGTCGGTAAGCATA
```

## Running translatorator

`translatorator` has two modes.  If `-mode asm` is specified, `translatorator` assumes that each input sequence (or read) is a fully correct / assembled sequence and proceeds accordingly.  The file specified by `-output-sequences` will then detail the sub-sequences of each input sequence (or read) that were matched by the kmers.


### Getting help

Use `translatorator -help` to see all options.

### Kmer matching notes

`translatorator` only considers an input sequence to match a set of kmers `(k1,k2), (k3,k4), ...` if

* there's a unique way to match k1, k2, ... to the sequence (allowing for IUPAC codes) in the given order; and
* the matching locations of k1, k2, ... don't overlap.


### Basic example 1:

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
```
# Written by translatorator
# Kmer pairs are:
# 1: CGTG / GTCG
# 2: GTAA / CATA
file    read_id strand  start_1 end_1   start_2 end_2   dna_sequence    aa_sequence
mysequence      my_sequence     +       2       15      18      27      CGTGGTCACCGTCGGTAAGTCATA        RGHRRXVI
```

### HiFi clustering example

Suppose you have an input data from a PacBio HiFi experiment in `myreads.bam` in a diploid organism (like humans) and
want to work out the coding sequence of a gene in the sample.

If the gene is located at `chr1:1000-5000` and the gene has two coding sequence records starting `CGAA`-`GCAA` and
`CGAT`-`GCAT`, you might run:

```
translatorator \
-mode hifi
-sequences myreads.bam \
-range chr1:1000-5000 \
-cds-kmers CGAA GCAA CGAT GTGA \
-output-clusters clusters.tsv
```

The output file would look like this:
```
# Written by translatorator
# Kmer pairs are:
# 1: CGAA / GCAA
# 2: CGAT / GTGA
file     type  cluster_id  hpc_id  supporting_reads  exact_reads     total_informative_reads   proportion   total_reads   sequence
myreads  dna   0                0                52           48                         112        0.464           118   CGAATTTTGCAACGATGGGGGTGA
myreads  dna   1                1                60           56                         112        0.536           118   CGAATAATGCAACGATGGGGATGA
myreads   aa   0                0               112          104                         112            1           118   RILQRWGH
```

This reflects that
- this sample looks like it is heterozygous for genetic variants in this gene (about half the reads support one haplotype, and half the other)
- these two haplotypes, however, have the same translation to amino acids

Under the hood, `translatorator` has:

* Searched for kmer matches in the reads and computed the matching sequences (use `-output-sequences` to see the detail.)
* Compressed homopolymers from the reads (to avoid confounding by PacBio HiFi homopolymer errors)
* Found the unique matched sequences and aligned all reads back to them (use `-output-identities` to see the results of alignment.)
* Filtered unique sequences down to a candidate list, based on those that are supported by multiple reads (controlled by ` -min-obs-per-sample` and `-min-fraction-per-sample`.)
* Assigned each read to one of the candidate sequences (controlled by `-min-alignment-identity`)
* Identified a consensus sequence for each candidate (currently by taking the most common sequence assigned to it).
* Output the results.
