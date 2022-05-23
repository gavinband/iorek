# tabulate-mismatches

`tabulate-mismatches` is a tool to help with context-specific error rate estimation for next generation sequencing datasets.

It:

* reads in read alignments from a BAM or CRAM file;
* reads the reference FASTA file to which the reads are aligned;
* then it walks the reads to find all mismatches, insertions and deletions, and tabulates them along with the bases involved, and the flanking sequence.

`tabulate-mismatches` can also keep track of genomic region annotations supplied from a seperate
annotation file, such as homopolymer and short repeat tracts computed using
[`find-homopolymers`](find-homopolymers.md).

## Synopsis

Get program options:

```
$ tabulate-mismatches -help

Welcome to tabulate-mismatches
(version: 0.7, revision f5d9393)

(C) 2009-2022 University of Oxford

Usage: tabulate-mismatches <options>

OPTIONS:
Input / output file options:
             -o <a>: Path of output file.  Defaults to "-".
         -range <a>: Genomic regions (expressed in the form <chromosome>:<start>-<end>) to process.  Regions are expres-
                     sed in 1-based, right-closed coordinates. (These regions should have few copy number variants) Alte-
                     rnatively this can be the name of a file containing a list of regions.
  -reads <a> <b>...: Path of bam/cram files to operate on.
     -reference <a>: Specify reference sequence
    -annotation <a>: Specify a BED file or text file of annotations (for example, of homopolymer tracts.) If a .txt file,
                     it must have columns "contig_id", "start", "end", "repeat", "length" using a 1-based, closed interv-
                     al coordinate system. If a BED file, it must have 4 columns with no headers, representing sequence 
                     ID, start, end, and annotation, using a 0-based, right-open coordinate system.

Model options:
       -by-position: Specify that errors should be tabulated by position, not aggregated.
            -mq <a>: Ignore alignments below this mapping quality threshold  Defaults to "0".
```

## Example

Some example data can be found in the `example/mismatches/` folder.

```
$ cat example.fa
>contig1
ATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCG
```

```
$ cat contig1_reads.sam
@HD	VN:1.6	SO:coordinate
@SQ	SN:contig1	LN:64
read1	0	contig1	1	60	4M	*	0	4	ATCG	*	XC:Z:four base match
read2	0	contig1	5	60	4M	*	0	4	ATCG	*	XC:Z:four base match
read3	0	contig1	5	60	4M	*	0	4	AtCG	*	XC:Z:four base match w/lower case base
read4	0	contig1	5	60	4M	*	0	4	TTCG	*	XC:Z:1st base mismatch, position 5
read5	0	contig1	5	60	4M	*	0	4	AACG	*	XC:Z:2nd base mismatch, position 6
read6	0	contig1	5	60	4M	*	0	4	ATTG	*	XC:Z:3rd base mismatch, position 7
read7	0	contig1	5	60	4M	*	0	4	ATCT	*	XC:Z:4th base mismatch, position 8
read8	0	contig1	11	60	1S3M	*	0	4	GCGA	*	XC:Z:clipped initial base
read9	0	contig1	11	60	1S3M	*	0	4	GCGT	*	XC:Z:clipped initial base w/mismatch at position 13
read10	0	contig1	13	60	3M1S	*	0	4	ATCC	*	XC:Z:clipped terminal base
read11	0	contig1	13	60	3M1S	*	0	4	TTCC	*	XC:Z:clipped terminal base w/mismatch at position 13
read12	0	contig1	17	60	2M1D1M	*	0	4	ATG	*	XC:Z:interior deletion at position 15
read13	0	contig1	21	60	2M1I2M	*	0	4	ATGCG	*	XC:Z:interior insertion at position 23
read14	0	contig1	25	60	3M2I5M	*	0	8	ATGAAGATCG	*	XC:Z:adjacent mismatch and insertion at position 27-28
read15	0	contig1	25	60	3M2D3M	*	0	8	ATGTCG	*	XC:Z:adjacent mismatch and deletion at position 27-28
read16	0	contig1	30	60	4M	*	0	4	TCGT	*	XC:Z:4th base mismatch, position 33
read17	0	contig1	31	60	4M	*	0	4	CGTT	*	XC:Z:3rd base mismatch, position 33
read18	0	contig1	32	60	4M	*	0	4	GTTC	*	XC:Z:2nd base mismatch, position 33
read19	0	contig1	33	60	4M	*	0	4	TTCG	*	XC:Z:another 1st base mismatch, position 33
```

```
$ tabulate-mismatches -reads contig1_reads.bam -reference example.fa

count	type	contig_sequence	read_sequence	left_flank	right_flank
1	D	C		GAT	GAT
1	D	GA		ATC	TCG
1	I		AA	ATC	GAT
1	I		G	GAT	CGA
7	X	A	T	TCG	TCG
2	X	C	G	GAT	GAT
1	X	C	T	GAT	GAT
1	X	G	T	ATC	ATC
1	X	T	A	CGA	CGA
```

Here (X = mismatch, I = insertion, D = deletion.

`tabulate-mismatches` can also tabulate by aligned position - this is in particular helpful for debugging:

```
$ tabulate-mismatches -reads contig1_reads.bam -reference example.fa -by-position

count	contig_id	position	type	contig_sequence	read_sequence	left_flank	right_flank
1	contig1	5	X	A	T	TCG	TCG
1	contig1	6	X	T	A	CGA	CGA
1	contig1	7	X	C	T	GAT	GAT
1	contig1	8	X	G	T	ATC	ATC
2	contig1	13	X	A	T	TCG	TCG
1	contig1	19	D	C		GAT	GAT
1	contig1	23	I		G	GAT	CGA
2	contig1	27	X	C	G	GAT	GAT
1	contig1	28	D	GA		ATC	TCG
1	contig1	28	I		AA	ATC	GAT
4	contig1	33	X	A	T	TCG	TCG
```

## Example of annotating with respect to repeat tracts

`tabulate-mismatches` can keep track of mismatches with respect to externally-supplied genomic
region annotations. The intended use case is to track error locations within homopolymers and other
short repeat tracts. Motivated by this, at present `tabulate-mismatches` only outputs the longest
two annotations overlapping any mismatch (as it's possible for a base to be in two repeat segments
at once).  

**Note**. Currently this tracking is based on the position of the mismatch. For deletions this is
the leftmost location of the deletion; a TODO is to find all overlapping annotations.

Here's an example:

```
$ cat contig2_reads.sam
@HD	VN:1.6	SO:coordinate
@SQ	SN:contig2	LN:57
read20	0	contig2	3	60	3M1I5M	*	0	8	TGAAACTGA	*	XC:Z:insertion in homopolymer, position 6
read21	0	contig2	9	60	2M1D5M	*	0	7	GAACTGA	*	XC:Z:deletion in homopolymer, position 11
read22	0	contig2	15	60	8M	*	0	7	GACAACTG	*	XC:Z:SNP in homopolymer, position 17
read23	0	contig2	22	60	3M2D3M	*	0	7	GACTGA	*	XC:Z:deletion in dinucleotide repeat, position 25
read24	0	contig2	22	60	8M	*	0	7	GACAGTGA	*	XC:Z:SNP in dinucleotide repeat, position 26
read25	0	contig2	22	60	5M2I3M	*	0	7	GACACACTGA	*	XC:Z:insertion in dinucleotide repeat, position 27
read26	0	contig2	35	60	4M3D1M	*	0	7	TGACT	*	XC:Z:deletion in trinucleotide repeat, position 39
read27	0	contig2	35	60	7M3I1M	*	0	7	TGACGACGACT	*	XC:Z:insertion in trinucleotide repeat, position 42
read28	0	contig2	35	60	8M	*	0	7	TGACGAGT	*	XC:Z:SNP in trinucleotide repeat, position 41
read29	0	contig2	48	60	9M	*	0	7	GAGACTACT	*	XC:Z:SNP in repeat, position 50
read30	0	contig2	48	60	9M	*	0	7	GACGCTACT	*	XC:Z:SNP in two repeats, position 51
read31	0	contig2	48	60	9M	*	0	7	GACAGTACT	*	XC:Z:SNP in two repeats, position 52
read32	0	contig2	48	60	9M	*	0	7	GACACGACT	*	XC:Z:SNP in repeat, position 53
```

Annotations can be passed in [BED4 format](https://en.wikipedia.org/wiki/BED_(file_format)) (i.e.
four columns specifying contig, start, end and annotation detail, using a 0-based, right-open
coordinate system.)  Alternatively they can be listed in the `find-homopolymers` output format as follows:

```
$ cat homopolymers.tsv
\# Computed by find-homopolymers 2022-05-22 23:04:49
\# Coordinates are 1-based, closed.
sequence_id	start	end	repeat	length
contig2	5	6	A	2
contig2	10	12	A	3
contig2	16	19	A	4
contig2	23	26	AC	4
contig2	28	31	GA	4
contig2	31	34	AC	4
contig2	36	41	GAC	6
contig2	49	52	AC	4
contig2	51	56	ACT	6
```

**Note.** This format assumes a 1-based, closed coordinate system.

Given the above, `tabulate-mismatches` will output seperate rows for mismatches occuring in each annoted segment:

```
$ tabulate-mismatches -reads contig1_reads.bam -reference example.fa -annotation homopolymers.tsv -by-position

count	contig_id	position	type	contig_sequence	read_sequence	left_flank	right_flank	annotation1	annotation1_length	annotation2	annotation2_length
1	contig2	6	I		A	TGA	ACT	A	2	NA	NA
1	contig2	11	D	A		TGA	ACT	A	3	NA	NA
1	contig2	17	X	A	C	TGA	AAC	A	4	NA	NA
1	contig2	25	D	AC		GAC	TGA	AC	4	NA	NA
1	contig2	26	X	C	G	ACA	TGA	AC	4	NA	NA
1	contig2	27	I		AC	CAC	TGA	NA	NA	NA	NA
1	contig2	39	D	GAC		GAC	TGA	GAC	6	NA	NA
1	contig2	41	X	C	G	CGA	TGA	GAC	6	NA	NA
1	contig2	42	I		GAC	GAC	TGA	NA	NA	NA	NA
1	contig2	50	X	C	G	GGA	ACT	AC	4	NA	NA
1	contig2	51	X	A	G	GAC	CTA	AC	4	ACT	6
1	contig2	52	X	C	G	ACA	TAC	AC	4	ACT	6
1	contig2	53	X	T	G	CAC	ACT	ACT	6	NA	NA

```

