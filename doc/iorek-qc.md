# iorek-qc

`iorek-qc` is traverses a set of reads in a fastq file and computes quality control (QC) metrics.  The metrics reported are:

- read length
- numbers of bases meeting different reported base quality levels
- overall measures of read quality, such as average base quality
- optionally, kmer-based measures of accuracy in reference to an external dataset of 'truth' kmers.

`iorek-qc` can output results for each read as well as acveraged for each position across reads.

Additionally, it has functionality to 'tag' reads with user-specified values, such that the results will be separated by tag.  We have found this useful for assessing read filtering strategies.

