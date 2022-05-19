# tabulate-mismatches

`tabulate-mismatches` is a tool to help with context-specific error rate estimation for next generation sequencing datasets.

It:

* reads in read alignments from a BAM or CRAM file;
* reads the reference FASTA file to which the reads are aligned;
* then it walks the reads to find all mismatches, insertions and deletions, and tabulates them along with the bases involved, and the flanking sequence.

In principle other information such as the location within homopolymer runs will be collected / tabulated, but this is not implemented yet.

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
         -range <a>: Genommic regions (expressed in the form <chromosome>:<start>-<end>) to process.  Regions are expres-
                     sed in 1-based, right-closed coordinates. (These regions should have few copy number variants) Alte-
                     rnatively this can be the name of a file containing a list of regions.
  -reads <a> <b>...: Path of bam/cram files to operate on.
     -reference <a>: Specify reference sequence

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
$ cat reads.sam
@HD	VN:1.6	SO:coordinate
@SQ	SN:contig1	LN:64
read1	0	contig1	1	60	4M	*	0	4	ATCG	*	CM:Z:four base match
read2	0	contig1	5	60	4M	*	0	4	ATCG	*	CM:Z:four base match
read3	0	contig1	5	60	4M	*	0	4	AtCG	*	CM:Z:four base match w/lower case base
read4	0	contig1	5	60	4M	*	0	4	TTCG	*	CM:Z:1st base mismatch, position 5
read5	0	contig1	5	60	4M	*	0	4	AACG	*	CM:Z:2nd base mismatch, position 6
read6	0	contig1	5	60	4M	*	0	4	ATTG	*	CM:Z:3rd base mismatch, position 7
read7	0	contig1	5	60	4M	*	0	4	ATCT	*	CM:Z:4th base mismatch, position 8
read8	0	contig1	11	60	1S3M	*	0	4	GCGA	*	CM:Z:clipped initial base
read9	0	contig1	11	60	1S3M	*	0	4	GCGT	*	CM:Z:clipped initial base w/mismatch at position 13
read10	0	contig1	13	60	3M1S	*	0	4	ATCC	*	CM:Z:clipped terminal base
read11	0	contig1	13	60	3M1S	*	0	4	TTCC	*	CM:Z:clipped terminal base w/mismatch at position 13
read12	0	contig1	17	60	2M1D1M	*	0	4	ATG	*	CM:Z:interior deletion at position 15
read13	0	contig1	21	60	2M1I2M	*	0	4	ATGCG	*	CM:Z:interior insertion at position 23
read14	0	contig1	25	60	3M2I5M	*	0	8	ATGAAGATCG	*	CM:Z:adjacent mismatch and insertion at position 27-28
read15	0	contig1	25	60	3M2D3M	*	0	8	ATGTCG	*	CM:Z:adjacent mismatch and deletion at position 27-28
read16	0	contig1	30	60	4M	*	0	4	TCGT	*	CM:Z:4th base mismatch, position 33
read17	0	contig1	31	60	4M	*	0	4	CGTT	*	CM:Z:3rd base mismatch, position 33
read18	0	contig1	32	60	4M	*	0	4	GTTC	*	CM:Z:2nd base mismatch, position 33
read19	0	contig1	33	60	4M	*	0	4	TTCG	*	CM:Z:another 1st base mismatch, position 33
```

```
$ tabulate-mismatches -reads example/mismatches/reads.bam -reference example/mismatches/example.fa

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
$ tabulate-mismatches -reads example/mismatches/reads.bam -reference example/mismatches/example.fa -by-position

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
