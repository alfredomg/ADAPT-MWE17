#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: <expe dir>" 1>&2
    echo "" 1>&2
    echo "where <expe dir> is the directory containing folders train and test, i.e." 1>&2 
    echo "either the fold number of the full-train-test dir." 1>&2
    exit 1
fi
dir="$1"

rm -f "$1"/weka.model "$1"/test/features-h.arff "$1"/test/features-h.tsv "$1"/test/features-h.withId.tsv "$1"/test/ids.tsv "$1"/test/output.parseme "$1"/test/output.reranked "$1"/test/predicted.arff "$1"/test/predicted.noId.tsv "$1"/test/predicted.tsv "$1"/train/features-h.arff "$1"/train/features-h.tsv "$1"/train/features-h.withId.tsv "$1"/train/ids.tsv "$1"/train/selfpredicted.arff "$1"/train/selfpredicted.noId.tsv "$1"/train/selfpredicted.tsv
