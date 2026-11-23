# translatorator

`translatorator` is a tool to locate subsequences of assembly-level DNA sequences or sequencing reads based
on the presence of specific kmers, or kmer patterns.  

A key use case is extracting the coding sequence of a gene based on kmers identifying the start and end of each exon.

`translatorator`'s particular USP is that it looks up sub-sequences based on pairs of DNA kmers at the 5' and 3' end.  The
kmers can be specified on the command line and have a particular syntax that allows for various levels of
flexibility that we've found useful.  Once found, the relevant portion(s) of the read are extracted and optionally 'translated'
to form predicted protein sequence.

**Note** `translatorator` is under development and its feature set, design, and usage might change.

## Diagram

With input conceptually like this:
```
read1 sequence  GCACGTGGTTCACCGTCGTCGTAAGTCATAC
read2 sequence  GCACGTGGT-CACCGTCGTCGTAAGTCATA
read3 sequence   CACGTGGT-CACCGTCGTCGTAAGTCATACGCC
read4 sequence   CACGTGGT-CACCGTCGTCGTAAGTCATACGCCGGC
read5 sequence    ACGTGGT-CACCGTCGTTGTAAG-CATA
read6 sequence    ACGTGGT-CACCGTCGTTGTAAG-CATACGCCGG

# kmer pairs:   ...CGTG.......GTCG..GTAA..CATA
```

And using kmer pairs `CGTG` - `GTCG`, and `GTAA` - `CATA`,  you get output conceptually like this for `-mode asm`:
```
 read sequence
read1 CGTGGTTCACCGTCGGTAAGTCATA
read2 CGTGGTCACCGTCGGTAAGTCATA
read3 CGTGGTCACCGTCGGTAAGTCATA
read4 CGTGGTCACCGTCGGTAAGTCATA
read5 CGTGGTCACCGTCGGTAAGCATA
read6 CGTGGTCACCGTCGGTAAGCATA
```

with one resulting extracted sequence per input sequence).

For `-mode hifi` you instead get this:
```
cluster number_of_reads total_informative_reads exact_reads proportion sequence
      0               4                       6           3 0.666      CGTGGTCACCGTCGGTAAGTCATA
      1               2                       6           2 0.333      CGTGGTCACCGTCGGTAAGCATA
```

with the resulting extracted sequences clustered and counted.  In this toy example, note that `read1` has
been clustered with `read2` and others similar reads.  `translatorator` has determined that, among reads with substantial frequency,
it is most similar to the first cluster.

## Features:

* Can work with FASTA input or with aligned BAM/SAM/CRAM files
* Finds sub-sequences of input sequences based on the presence of specific kmer pairs in the specific order given
* In `asm` mode, outputs one sequence per input sequence (provided the kmers match).
* In `hifi` mode, allows for read errors and so outputs a representation of the sequences that are in the sample and their frequency amongst the reads.
* Examines sequences in both forward and reverse orientation
* Allows for IUPAC codes in specified kmers, to allow for known genetic variants
* Outputs matching sequences and their translation into amino acid sequence

## Running translatorator

`translatorator` has two modes.  

### Assembly sequence mode

If `-mode asm` is specified, `translatorator` assumes that each input sequence (or read) is a fully correct / assembled sequence and proceeds accordingly.  The file specified by `-output-sequences` will then detail the sub-sequences of each input sequence (or read) that were matched by the kmers.

### Sequence read mode


If `-mode hifi` is specified, `translatorator` assumes that the input reads are PacBio 'hifi' reads and proceeds accordingly:

* Each read is examined for the presence of the kmer patterns (up to matching options).  If uniquely present in the right order, the relevant part of the sequence is extracted.  The original coordinates of the segments within the reads are retained for later use.
* Reads undergo homopolymer compression (within each contiguous segment).
* Homopolymer compressed reads meeting user-specified frequency filters are retained and treated as candidate haplotypes.
* All homopolymers-compressed reads are re-aligned to the candidates.  Each read is assigned to the candidate with the highest alignment identity, provided this meets a user-specified threshold.
* The number of reads aligning to each candidate is counted
* The candidates are output along with their counts
* Finally, the candidates are also 'translated' into amino acid sequences.

The file specified by `-output-sequences` will then detail the sub-sequences of each input sequence (or read) that were matched by the kmers.


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
will figure out that the input sequence segments like this:

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
