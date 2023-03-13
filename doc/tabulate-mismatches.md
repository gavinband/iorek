# tabulate-mismatches

`tabulate-mismatches` traverses a set of read alignments and counts matched, mismatched, inserted and deleted bases, according to the bases involved and the sequence context.  The main use case is estimating sequencing accuracy.  Specifically it:

* reads in read alignments from a BAM or CRAM file;
* reads the reference FASTA file to which the reads are aligned;
* then it walks the reads to find all matches, mismatches, insertions and deletions, and tabulates them according to sequence context.

The output is a large table with counts of each 'event' (match, mismatch, insertion or deletion relative to the reference) separated by the read and contig bases involved and the flanking sequence.  Additionally `tabulate-mismatches` is able to mask out parts of the genome to ignore in this analysis.

## Synopsis

Get program options:

```
$ tabulate-mismatches -help

Welcome to tabulate-mismatches
(version: 0.9, revision 317ef9b)

(C) 2009-2023 University of Oxford

Usage: tabulate-mismatches <options>

OPTIONS:
Input / output file options:
         -fasta <a>: Specify reference sequence
          -mask <a>: Specify a BED file of regions to mask out of the analysis. The file should have no column names and
                     should have contig, start and end columns,  expressed in 0-based right-closed form.
             -o <a>: Path of output file.  Defaults to "-".
         -range <a>: Genomic regions (expressed in the form <chromosome>:<start>-<end>) to process.  Regions are express-
                     ed in 1-based, right-closed coordinates. (These regions should have few copy number variants) Alter-
                     natively this can be the name of a file containing a list of regions.
  -reads <a> <b>...: Path of bam/cram files to operate on.

Model options:
       -by-position: Specify that errors should be tabulated by position, not aggregated.
         -flank <a>: Specify how much flanking sequence to classify by.  Defaults to "3".
            -mq <a>: Ignore alignments below this mapping quality threshold  Defaults to "20".

```

### Basic usage

The basic use is:
```
$ tabulate-mismatch -reads <path to one or more .bam/.cram files> -fasta <path to reference FASTA file> [-o <path to output file>]
```

**Note**. The reference FASTA file and the bam/cram files must match each other.  In particular all contig IDs in the read files must be present in the FASTA
file, otherwise the program will halt with an error.

The program prints output to stdout, or (if `-o` is given) to the specified output file.
The output file format depends on the filename - if it ends in `.csv` or `.tsv` a comma- or tab-delimited file will be output, if `.txt` a space-delimited file will be output.  The default is tab-delimited.  An output file with the `.gz` extension will be gzip-compressed.

## Example

### Example data

Some example data can be found in the `example/mismatches/` folder.

```
$ cat example.fa
>contig1
ATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATCG
>contig2
ACTGAACTGAAACTGAAAACTGACACTGAGACACTGACGACTGACTGGACACTACT
```

```
% cat contig1_reads.sam 
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
read19	0	contig1	33	60	4M	*	0	4	TTCG	*	XC:Z:another 1st base mismatch, position 33%
```

The total number of reference bases sequenced here is 80, and the total number of 'error' bases is 18
(12 mismatches, 3 deleted bases, and 3 inserted bases) as counted from the comments.

### Running the program

`tabulate-mismatches` can now be run like this:

```
$ tabulate-mismatches -reads contig1_reads.bam -fasta example.fa
```

which produces:
```
count	type	contig_sequence	read_sequence	left_flank	right_flank	contig_bases_unmasked	edit_bases_unmasked
1	=	A	A		TCG	1	0
12	=	A	A	TCG	TCG	1	0
1	=	C	C	AT	GAT	1	0
16	=	C	C	GAT	GAT	1	0
17	=	G	G	ATC	ATC	1	0
1	=	T	T	A	CGA	1	0
17	=	T	T	CGA	CGA	1	0
1	D	C		GAT	GAT	1	1
1	D	GA		ATC	TCG	2	2
1	I		AA	ATC	GAT	0	2
1	I		G	GAT	CGA	0	1
7	X	A	T	TCG	TCG	1	1
2	X	C	G	GAT	GAT	1	1
1	X	C	T	GAT	GAT	1	1
1	X	G	T	ATC	ATC	1	1
1	X	T	A	CGA	CGA	1	1
```

In total this shows: 12 mismatches, 2 deletions, and 2 insertions; a total of 80 contig bases and 18 edit bases,
consistent with what is shown in the read alignment file.

### Understanding the output

The output columns are:

1. the `count`, that is, the number of times this event was seen in the alignments.
2. the `type` of the event.  It is either `=` (match), `X` (mismatch), `I` (insertion) or `D` (deletion).
3. the `contig_sequence` i.e. the sequence of bases on the reference FASTA file that were affected/covered by this event
4. the `read_sequence` i.e. the sequence of bases on the read that were affected/involved by this event
5. the `left_flank`, i.e. the bases in the contig sequence immediately to the left of those affected/covered by the event
6. the `right_flank`, i.e. the bases in the contig sequence immediately to the right of those affected/covered by the event
7. `contig_bases_unmasked`, the number of reference contig bases affected
8. `edit_bases_unmasked`, the edit distance from the reference contig to the read sequence for this event

### Computing error rates

A natural measure of error rate is the average number of error bases, per base sequenced:

EPB = (mismatching bases + inserted bases + deleted bases) / (total bases sequenced)

If the input read alignments represent accurate aligments between some reads and the true underlying genome, the EPB can be estimated as:

EPB ~ sum( edit_bases_unmasked * count ) / sum(contig_bases_unmasked * count)

Here, `contig_bases_unmasked` represents the total number of bases actually sequenced for each event, while `edit_bases_unmasked` represents the total number of error bases.  

### Masking the genome

In many situations the true genome won't be known beforehand.  To handle this situation, `tabulate-mismatches` has a `-mask` option which allows to only focus the analysis on a specific part of the genome.  The argument is a BED file containing intervals that should be masked out.  The BED file should contain three (unnamed) columns representing the contig id, start and end coordinates of each masked region.  Coordinates in the file are in 0-based, half-closed coordinate system.

If `-mask` is specified, `tabulate-mismatches` still behaves the same way, except that for each event in the CIGAR string, it additionally computes the number of unmasked reference bases involved and the number of unmasked edit bases involved.  For example, consider these alignments:

```
REFERENCE:  ...ACGTACGTACGTACG---TACG...    
     MASK:     *****              ***
    READS: 1:   TGT--GTACG                
           2:          A--TGCGGGGTA       
           3:            G-ACG---TGCG...     
```

The table should look like this:

| read  | contig sequence | read sequence | contig bases unmasked | edit bases unmasked |
-----------------------------------------------------------------------------------------
| 1     | C               | T             | 0                     | 0                   |
| 1     | AC              | --            | 1                     | 1                   |
| 2     | A               | G             | 1                     | 1                   |
| 2     | CG              | --            | 2                     | 2                   |
| 2     | ---             | GGG           | 0                     | 3                   |
| 3     | T               | -             | 1                     | 1                   |
| 3     | A               | G             | 0                     | 0                   |

Since:

- read 1 contains one mismatch and one deletion, which overlaps the unmasked region by one base.
- read 2 contains one deletion, one mismatch, and one insertion, none of which overlap the mask
- read 3 contains one unmasked deletion, and one mismatch that overlaps the mask.

### Tracking mismatches by position

`tabulate-mismatches` can also tabulate by aligned position - this is in particular helpful for debugging.
For example the full list of matches, mismatches, insertions and deletions in the above data can be output like this:

```
$ tabulate-mismatches -reads contig1_reads.bam -fasta example.fa -by-position
```

...which produces:
```
count   contig_id       position        type    contig_sequence read_sequence   left_flank      right_flank     contig_bases_unmasked   edit_bases_unmasked
1       contig1 1       =       A       A               TCG     1       0
1       contig1 2       =       T       T       A       CGA     1       0
1       contig1 3       =       C       C       AT      GAT     1       0
1       contig1 4       =       G       G       ATC     ATC     1       0
5       contig1 5       =       A       A       TCG     TCG     1       0
1       contig1 5       X       A       T       TCG     TCG     1       1
5       contig1 6       =       T       T       CGA     CGA     1       0
1       contig1 6       X       T       A       CGA     CGA     1       1
5       contig1 7       =       C       C       GAT     GAT     1       0
1       contig1 7       X       C       T       GAT     GAT     1       1
5       contig1 8       =       G       G       ATC     ATC     1       0
1       contig1 8       X       G       T       ATC     ATC     1       1
2       contig1 11      =       C       C       GAT     GAT     1       0
2       contig1 12      =       G       G       ATC     ATC     1       0
2       contig1 13      =       A       A       TCG     TCG     1       0
2       contig1 13      X       A       T       TCG     TCG     1       1
2       contig1 14      =       T       T       CGA     CGA     1       0
2       contig1 15      =       C       C       GAT     GAT     1       0
1       contig1 17      =       A       A       TCG     TCG     1       0
1       contig1 18      =       T       T       CGA     CGA     1       0
1       contig1 19      D       C               GAT     GAT     1       1
1       contig1 20      =       G       G       ATC     ATC     1       0
1       contig1 21      =       A       A       TCG     TCG     1       0
1       contig1 22      =       T       T       CGA     CGA     1       0
1       contig1 23      =       C       C       GAT     GAT     1       0
1       contig1 23      I               G       GAT     CGA     0       1
1       contig1 24      =       G       G       ATC     ATC     1       0
2       contig1 25      =       A       A       TCG     TCG     1       0
2       contig1 26      =       T       T       CGA     CGA     1       0
2       contig1 27      X       C       G       GAT     GAT     1       1
1       contig1 28      =       G       G       ATC     ATC     1       0
1       contig1 28      D       GA              ATC     TCG     2       2
1       contig1 28      I               AA      ATC     GAT     0       2
1       contig1 29      =       A       A       TCG     TCG     1       0
3       contig1 30      =       T       T       CGA     CGA     1       0
4       contig1 31      =       C       C       GAT     GAT     1       0
5       contig1 32      =       G       G       ATC     ATC     1       0
4       contig1 33      X       A       T       TCG     TCG     1       1
3       contig1 34      =       T       T       CGA     CGA     1       0
2       contig1 35      =       C       C       GAT     GAT     1       0
1       contig1 36      =       G       G       ATC     ATC     1       0
```

This again shows: 12 mismatches, 2 deletions, and 2 insertions; a total of 80 contig bases and 18 edit bases,
consistent with what is shown in the read alignment file.
