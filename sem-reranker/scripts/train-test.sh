#!/bin/bash



progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
featsOpts=
force=0

function usage {
  echo
  echo "Usage: $progName [options]  <work dir> <weka model> <corpus file> <weka params>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -l <train input> learn model from <train input>: output from the CRF"
  echo "       process with multiple solutions, with gold answer."
  echo "    -a <test input> apply model to <test input>: output from the CRF"
  echo "       process with multiple solutions, without gold answer."
  echo "    -o <options for features> options for the program context-features (quoted)"
  echo "    -f force recomputing features even if already there"
  echo "    -h this help"
  echo
}






OPTIND=1
while getopts 'hl:a:r:o:f' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) force=1;;
	"l" ) trainFile="$OPTARG";;
	"a" ) testFile="$OPTARG";;
	"o" ) featsOpts="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 4 ]; then
    echo "Error: expecting 4 args." 1>&2
    printHelp=1
fi
if [ -z "$trainFile" ] &&  [ -z "$testFile" ]; then
    echo "$progName error: one of -l and -a must be supplied." 1>&2
    printHelp=1
elif  [ ! -z "$trainFile" ] &&  [ ! -z "$testFile" ]; then
    echo "$progName error: cannot use both -l and -a." 1>&2
    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi
workDir="$1"
modelFile="$2"
epFile="$3"
wekaParams="$4"

    
    
[ -d "$workDir" ] || mkdir "$workDir"

if [ ! -z "$trainFile" ]; then # features for training
    if [ $force -eq 1 ] || [ ! -s "$workDir/train-features-h.withId.tsv" ]; then
	command="context-features-train $featsOpts \"$epFile\" \"$trainFile\" \"$workDir/train-features-h.withId.tsv\""
#	echo "DEBUG: $command"  1>&2
	eval "$command"
    fi
    cut -f 3- "$workDir/train-features-h.withId.tsv" >"$workDir/train-features-h.tsv"
    nbFeats=$(head -n 1 "$workDir/train-features-h.tsv" | wc -w)
    if [ $nbFeats -le 1 ]; then
	echo "Warning: no features at all, not computing weka model." 1>&2
	echo "nope" >"$modelFile"
    else
	cut -f 1,2 "$workDir/train-features-h.withId.tsv" | tail -n +2  >"$workDir/ids.tsv"
	cat "$workDir/train-features-h.tsv" | sed 's/NA/?/g' | convert-to-arff.pl >"$workDir/train-features-h.arff"
	weka-learn-and-apply.sh -k "$workDir/weka.output" -m "$modelFile" "$wekaParams" "$workDir/train-features-h.arff" "$workDir/train-features-h.arff" "$workDir/selfpredicted.arff"
	convert-from-arff.pl <"$workDir/selfpredicted.arff" >"$workDir/selfpredicted.noId.tsv"
	paste "$workDir/ids.tsv" "$workDir/selfpredicted.noId.tsv" >"$workDir/selfpredicted.tsv"
    fi
    
fi

if [ ! -z "$testFile" ]; then # features for testing
    if [ ! -s "$modelFile" ]; then
	echo "Error: weka model file '$modelFile' does not exist" 1>&2
	exit 1
    fi
    if [ $force -eq 1 ] || [ ! -s  "$workDir/test-features-h.withId.tsv" ]; then
	eval "context-features-test $featsOpts \"$epFile\" \"$testFile\" \"$workDir/test-features-h.withId.tsv\""
    fi
    cut -f 3- "$workDir/test-features-h.withId.tsv" >"$workDir/test-features-h.tsv"
    nbFeats=$(head -n 1 "$workDir/test-features-h.tsv" | wc -w)
    if [ $nbFeats -le 1 ]; then
	echo "Warning: no features at all, returning CRF predictions." 1>&2
	print=""
	cat "$testFile" | while read line; do # copy CRF output but only the first alternative no 0
	    if [ "${line:0:1}" == "#" ]; then
		if [ "${line:2:1}" == "0" ]; then
		    print="yep"
		else
		    print=""
		fi
	    fi
	    if [ ! -z "$print" ]; then
		echo "$line"
	    fi
	done > "$workDir/output.reranked"
    else
	cut -f 1,2 "$workDir/test-features-h.withId.tsv" | tail -n +2 >"$workDir/ids.tsv"
	cat "$workDir/test-features-h.tsv" | sed 's/NA/?/g' | convert-to-arff.pl >"$workDir/test-features-h.arff"
	weka-learn-and-apply.sh -a "$modelFile" "$wekaParams" UNUSED "$workDir/test-features-h.arff" "$workDir/predicted.arff"
	convert-from-arff.pl <"$workDir/predicted.arff" >"$workDir/predicted.noId.tsv"
	paste "$workDir/ids.tsv" "$workDir/predicted.noId.tsv" >"$workDir/predicted.tsv"
	coverageFeatColNo=$(head -n 1 "$workDir/test-features-h.withId.tsv" | tr '\t' '\n' | grep -n "freqExprRefCorpusMEAN" | cut -f 1 -d ':')
	myOpts="-c $coverageFeatColNo"
	if [ -z "$coverageFeatColNo" ]; then
	    echo "Warning: cannot find coverage feature column" 1>&2
	    myOpts=""
	fi
	if [ -f "$workDir/test-gold.bio" ]; then
	    myOpts="$myOpts -g \"$workDir/test-gold.bio\""
	fi
	eval "apply-predict.pl $myOpts -a \"$workDir/analysis.tsv\" \"$workDir/predicted.tsv\"  \"$testFile\" \"$workDir/output.reranked\""
    fi
    bio4eval.pl --input "$workDir/output.reranked" --output "$workDir/output.parsemetsv"
fi

