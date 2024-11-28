#!/bin/bash
../../build/apps/translatorator -sequences reads.bam -cds-kmers GTTAA AAAAA -range chr1:1-60
../../build/apps/translatorator -sequences reads.bam -cds-kmers TTTTT TTAAC -range chr1:1-60
../../build/apps/translatorator -sequences reads.fasta -cds-kmers GTTAA AAAAA -range chr1:1-60
../../build/apps/translatorator -sequences reads.fasta -cds-kmers TTTTT TTAAC -range chr1:1-60
../../build/apps/translatorator -cluster -sequences reads.bam -cds-kmers GTTAA AAAAA -range chr1:1-60 -summary - -min-fraction-per-sample 0 -min-obs-per-sample 1
