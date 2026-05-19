#!/bin/bash

mkdir -p tmp
rm -f tmp/PfCRT.sequences.tsv

#-sequences PfCRT.fa.gz \

../../build/apps/translatorator_v0.9.3 \
-sequences PfCRT.fa.gz \
-mode asm \
-kmer-pairs  \
ATGAAATTCGCAAGTAAAAAAAAT TTAGATAATTTAGTACAAGAAGGAA \
ATGGCTCACGTTTAGGTGGAGGT GGAAATAAAAAGGGAAATTCAAAA \
GAACGACACCGAAGCTTTAATTTA CTGCTTTTTAATATTAAGATATAG \
ATATCACTTATACAATTATCTCGGA CTTGTCTTAATTAGTNCCTTAATT \
CCTGTATGCTTTTCAAACATGACA AAGATTGACATTTTAAGATTAAAT \
GCTATGGTATCCTTTTTCCAATTG TACACCCTTCCATTTTTAAAANAAC \
TTCATTTACCATATAATGAAATA TGGGAAGAAACACAGTCGTAGAG \
AATTGTGGTCTTGGTATGGCTAAG TGTGATGATTGTGACGGAGCATGG \
AAAACCTTCGCATTGTTTTCCTTC TGTGATAATTTAATAANCAGCTAT \
ATTATCGACAAATTTTCTACCATG GCTTATTACTTTAAATTCTTAGCC \
GGTGATGTTGTAANAGAACCA AGATTATTAGATTTCGTAACTTTG \
TTTGGCTACCTATTTGGTTCTATA GTAGGAAATATTATCTTAGAAA \
GAAAAAAAATGAGAAATGAAGAA GTCGATTCAATTATTACACAATAA \
-summary - \
-min-fraction-per-sample 0 \
-min-obs-per-sample 1 \
-output-sequences tmp/PfCRT.sequences.tsv \
-o tmp/PfCRT.tsv \
-output-identities -

cat tmp/PfCRT.sequences.tsv | grep -v '^#' 
lines=`cat tmp/PfCRT.sequences.tsv | grep -v '^#' | grep "ATG" | wc -l | sed -e 's/[[:space:]]*//'`
lines2=`zgrep '^>' PfCRT.fa.gz | wc -l | sed -e 's/[[:space:]]*//'`
if [[ "${lines}" != "${lines2}" ]]; then
	echo "!! Wrong number of complete CRT lines detected in tmp/PfCRT.sequences.tsv ('${lines}' instead of '${lines2}')."
	exit -1
else
	echo "++ Found ${lines} complete lines in tmp/PfCRT.sequences.tsv, as expected."
fi

