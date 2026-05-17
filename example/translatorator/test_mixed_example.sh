#!/bin/bash
mkdir -p tmp
../../build/apps/translatorator_v0.9.2 \
-sequences mixed_example.fa \
-mode hifi \
-cluster \
-threads 1 \
-min-obs-per-sample 1 \
-iterations 10 \
-cds-kmers \
ATGGAAACTATACAAGAACTAATA \
TGCACCTTGTCGAAAAATATTTAG \
-output-sequences tmp/mixed_example.reads.tsv

