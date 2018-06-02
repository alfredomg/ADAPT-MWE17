#!/usr/bin/env bash

# Usage:
#   cv-runexps.sh cvLanguageDir evalDir templateFile CValue [crf_test_Options]
# Example:
#   cv-runexps.sh ~/code/repos/MWE17SharedTask/data/FR base.template 10.0

cvdatadir=$1
template=$2
cvalue=$3
crf_test_ops=$4

for fold in {0..4} ; 
do
    if [ -d "$cvdatadir/$fold" ] ; then
        echo "$fold"
        echo "$cvdatadir/$fold/rnd.bio.trn"
        crf_learn  -c $cvalue  $template  "$cvdatadir/$fold/rnd.bio.trn"  "model.crfpp.$fold"  || exit $?
        crf_test   -m model.crfpp.$fold $crf_test_ops  "$cvdatadir/$fold/rnd.bio.tst"  >  "rnd.bio.tst.predict.$fold"  || exit $?

        bio4eval.pl --input "rnd.bio.tst.predict.$fold"  --output "rnd.bio.tst.predict.$fold.ptsv"  || exit $?
        evaluate.py "$cvdatadir/$fold/rnd.parsemetsv.tst"  "rnd.bio.tst.predict.$fold.ptsv"  >  "eval.$fold"  || exit $?
        cat "eval.$fold"
    else
	echo "Error: dir '$cvdatadir/$fold' not found" 1>&2
    fi
done
