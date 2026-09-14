#!/bin/bash
MIN_EXACT="$1"
echo "min exact = ${MIN_EXACT}"

mkdir -p tmp
../../build/apps/translatorator_v0.9.6 \
-mode hifi \
-sequences EdT-Pon-nDi.bam \
-exclude-reads EdT-Pon-nDi.excluded_reads.txt \
-range Pf3D7_08_v3:546988-550644 \
-iterations 10 \
-truncate-at-stops \
-min-hpc-reads 5 \
-min-fraction-hpc-reads 0.05 \
-min-fraction-exact-reads ${MIN_EXACT} \
-kmer-pairs \
ATGGAAACTATACAAGAACTAATA \
TGCACCTTGTCGAAAAATATTTAG \
GAAAAATTATTAATACGTCCTACT \
TTTTATATCAAAAAAATATATGTG \
GTGGATTAGCAATTGCTTCCTACA \
TTAACAAAAATAGACCAAGTGTAA \
-o tmp/EdT-Pon-nDi.clusters.tsv \
-output-sequences tmp/EdT-Pon-nDi.reads.tsv \
-output-alignments tmp/EdT-Pon-nDi.alignments.tsv \
-output-identities tmp/EdT-Pon-nDi.identities.tsv

