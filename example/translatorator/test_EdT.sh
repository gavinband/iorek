#!/bin/bash
HPC_WEIGHT="$1"
if [[ $HPC_WEIGHT == "" ]]; then
HPC_WEIGHT="0.0"
fi

echo "HPC weight =  ${HPC_WEIGHT}"

mkdir -p tmp
../../build/apps/translatorator_v0.9.2 \
-mode hifi \
-sequences EdT-Pon-nDi.bam \
-exclude-reads EdT-Pon-nDi.excluded_reads.txt \
-range Pf3D7_08_v3:546988-550644 \
-homopolymer-indel-weight ${HPC_WEIGHT} \
-iterations 10 \
-truncate-at-stops \
-min-obs-per-sample 5 \
-min-fraction-per-sample 0.05 \
-cds-kmers \
ATGGAAACTATACAAGAACTAATA \
TGCACCTTGTCGAAAAATATTTAG \
GAAAAATTATTAATACGTCCTACT \
TTTTATATCAAAAAAATATATGTG \
GTGGATTAGCAATTGCTTCCTACA \
TTAACAAAAATAGACCAAGTGTAA \
-output-sequences tmp/EdT-Pon-nDi.reads.tsv \
-output-alignments tmp/EdT-Pon-nDi.alignments.tsv \
-output-clusters tmp/EdT-Pon-nDi.clusters.tsv \
-output-identities tmp/EdT-Pon-nDi.identities.tsv

../../build/apps/translatorator_v0.9.2 \
-mode hifi \
-cluster \
-sequences EdT-Pon-nDi.bam \
-exclude-reads EdT-Pon-nDi.excluded_reads.txt \
-range Pf3D7_08_v3:546988-550644 \
-homopolymer-indel-weight ${HPC_WEIGHT} \
-iterations 10 \
-truncate-at-stops \
-cds-kmers \
ATGGAAACTATACAAGAACTAATA \
TGCACCTTGTCGAAAAATATTTAG \
GAAAAATTATTAATACGTCCTACT \
TTTTATATCAAAAAAATATATGTG \
GTGGATTAGCAATTGCTTCCTACA \
TTAACAAAAATAGACCAAGTGTAA \
-output-sequences tmp/EdT-Pon-nDi.clustered.reads.tsv \
-output-alignments tmp/EdT-Pon-nDi.clustered.alignments.tsv \
-output-clusters tmp/EdT-Pon-nDi.clustered.clusters.tsv \
-output-identities tmp/EdT-Pon-nDi.clustered.identities.tsv

