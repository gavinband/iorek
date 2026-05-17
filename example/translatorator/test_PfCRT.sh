#!/bin/bash

mkdir -p tmp

../../build/apps/translatorator_v0.9.1 \
-sequences PfCRT.fa.gz \
-mode asm \
-cds-kmers  \
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
TTTGGCTACCTATTTGGTTCTATA GTAGGAAATATTATCTTAGAAAG \
GAAAAAAAATGAGAAATGAAGAA GTCGATTCAATTATTACACAATAA \
-summary - \
-min-fraction-per-sample 0 \
-min-obs-per-sample 1 \
-output-sequences tmp/PfCRT.sequences.tsv \
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

