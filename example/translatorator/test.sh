#!/bin/bash
../../build/apps/translatorator_v0.9.2 -sequences reads.bam -cds-kmers GTTAA AAAAA -range chr1:1-60
../../build/apps/translatorator_v0.9.2 -sequences reads.bam -cds-kmers TTTTT TTAAC -range chr1:1-60
../../build/apps/translatorator_v0.9.2 -sequences reads.fasta -cds-kmers GTTAA AAAAA -range chr1:1-60
../../build/apps/translatorator_v0.9.2 -sequences reads.fasta -cds-kmers TTTTT TTAAC -range chr1:1-60
../../build/apps/translatorator_v0.9.2 -cluster -sequences reads.bam -cds-kmers GTTAA AAAAA -range chr1:1-60 -summary - -min-fraction-per-sample 0 -min-obs-per-sample 1
