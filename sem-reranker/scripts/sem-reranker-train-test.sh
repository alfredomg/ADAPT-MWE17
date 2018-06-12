#!/bin/bash



progName=$(basename "$BASH_SOURCE")

trainFile=
testFile=
force=0

function usage {
  echo
  echo "Usage: $progName [options]  <config file> <ref data dir> <weka model> <work dir>"
  echo
  echo "  <ref data dir>  is a directory containing  the corpus to use as reference with the"
  echo "   semantic reranker. It can contain several files, the one to be used is specified"
  echo "   in the config file config file as: "
  echo "     corpusFile=<filename>[:lemma]"
  echo "   where the optional [:lemma] part indicates to use option -l with the semantic"
  echo "   reranker. <filename> can for instance be the same as the input training file,"
  echo "   or a tokenized version of Europarl."
  echo "   Remark: token is supposed to be in column 2 and lemma in column 4 for the"
  echo "            input data; a reference file has the token as 2nd column."
  echo "   Remark: <filename> must not contain the path."
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -l <train input> learn model from <train input>: output from the CRF"
  echo "       process with multiple solutions, with gold answer."
  echo "    -a <test input> apply model to <test input>: output from the CRF"
  echo "       process with multiple solutions, without gold answer."
#  echo "    -o <options for features> options for the program context-features (quoted)"
#  echo "    -f force recomputing features even if already there"
  echo "    -h this help"
  echo
}



function readFromParamFile {
    local file="$1"
    local name="$2"
    local errMsgPrefix="$3"
    local sepa="$4"
    local noWarningIfEmpty="$5" # left empty => warning if defined but empty value
    local returnThisValueIfNotDefined="$6" # left empty => error and exit if not defined
    local assignToThisNameInstead="$7" # if defined, $name is the name read from the file but the value is assigned to this name

    if [ -z "$sepa" ]; then
	sepa="="
    fi
    if [ ! -f  "$file" ]; then
	echo "Error: cannot open config file '$file'" 1>&2
	exit 3
    fi
    line=$(grep "^$name$sepa" "$file" | tail -n 1)
    if [ -z "$line" ]; then
	if [ -z "$returnThisValueIfNotDefined" ]; then
	    echo "${errMsgPrefix}Error: parameter '$name' not found in parameter file '$file'" 1>&2
	    exit 1
	else
	    res="$returnThisValueIfNotDefined"
	fi
    else
	res=$(echo "$line" | cut -d "$sepa" -f 2- )
	if [ -z "$noWarningIfEmpty" ] && [ -z "$res" ]; then
	    echo "${errMsgPrefix}Warning: parameter '$name' is defined but empty in parameter file '$file'" 1>&2
	fi
    fi
    if [ -z "$assignToThisNameInstead" ]; then
	eval "$name=\"$res\""
    else
	eval "$assignToThisNameInstead=\"$res\""
    fi
}





OPTIND=1
while getopts 'hl:a:r:f' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) force=1;;
	"l" ) trainFile="$OPTARG";;
	"a" ) testFile="$OPTARG";;
#	"o" ) featsOpts="$OPTARG";;
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
configFile="$1"
refDataDir="$2"
modelFile="$3"
workDir="$4"

[ -d "$workDir" ] || mkdir "$workDir"

if [ ! -f "$configFile" ]; then
    echo "Error: cannot open config file '$configFile'" 1>&2
    exit 2
fi
if [ ! -d "$refDataDir" ]; then
    echo "Error: no directory '$refDataDir' found" 1>&2
    exit 2
fi


featsOpts=""
readFromParamFile "$configFile" "refCorpus"
refCorpusOpt=${refCorpus%:lemma}
if [ "$refCorpusOpt" != "$refCorpus" ]; then
    refCorpus=$refCorpusOpt
    featsOpts="$featsOpts -l 1"
else
    featsOpts="$featsOpts -l 0"
fi
corpusFile="$refDataDir/$refCorpus"

if [ "$refCorpus" == "NONE" ]; then
    echo "Error: refCorpus set to NONE, aborting" 1>&2
    exit 6
fi
if [ ! -f "$corpusFile" ]; then
    echo "Error: no reference data $corpusFile" 1>&2
    exit 6
fi

readFromParamFile "$configFile" "learnMethod"
wekaParams=$(weka-id-to-parameters.sh "$learnMethod")

readFromParamFile "$configFile" minFreq
readFromParamFile "$configFile" nMostFreq
readFromParamFile "$configFile" halfWindowSize
readFromParamFile "$configFile" normalizeContexts
readFromParamFile "$configFile" simMeasure
readFromParamFile "$configFile" minConfidence
readFromParamFile "$configFile" onlyMean
readFromParamFile "$configFile" windowIncludeWordsINexpr
readFromParamFile "$configFile" windowMultipleOccurrencesPosition
readFromParamFile "$configFile" featConfidence
readFromParamFile "$configFile" featGroups
readFromParamFile "$configFile" featTypes
readFromParamFile "$configFile" featStd


featsOpts="$featsOpts -m $minFreq -n $nMostFreq -w $halfWindowSize -c $normalizeContexts -s $simMeasure -p $minConfidence -o $onlyMean -a $windowIncludeWordsINexpr -b $windowMultipleOccurrencesPosition -C $featConfidence -g $featGroups -t $featTypes -f $featStd"
    
    

if [ ! -z "$trainFile" ]; then # features for training
    if [ $force -eq 1 ] || [ ! -s "$workDir/train-features-h.withId.tsv" ]; then
	command="context-features-train $featsOpts \"$corpusFile\" \"$trainFile\" \"$workDir/train-features-h.withId.tsv\""
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
	eval "context-features-test $featsOpts \"$corpusFile\" \"$testFile\" \"$workDir/test-features-h.withId.tsv\""
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

