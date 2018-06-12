#!/bin/bash


progName=$(basename "$BASH_SOURCE")

forceOpt=""

dataFilePrefix="train"

function usage {
  echo
  echo "Usage: $progName [options] <data dir> <config file> <template dir> <ref data dir> <output dir>"
  echo
  echo "  <data dir> must contain files train.parsemetsv and train.conllu, the two data "
  echo "  files used in the cross-validation process. See also -n"
  echo
  echo "  See run-config-train-test.sh for explanations about the parameter <template dir>."
  echo "  See sem-reranker-train-test.sh for explanations about the parameter <ref data dir>."
  echo
  echo "  Options:"
  echo "    -h this help"
#  echo "    -l <train input dir> learn model from <train input dir>, which must contain"
#  echo "       files train.parsemetsv and train.conllu."
#  echo "    -a <test input dir> apply model to <test input dir> which must contain"
#  echo "       files test.parsemetsv and test.conllu."
#  echo "    -o <options for features> options for the program context-features (quoted)"
  echo "    -f force recomputing features even if already there"
  echo "     -n <filename prefix> to use instead of 'train'."
  echo
}




#twdoc readFromParamFile $file $name $errMsgPrefix [$separator] [$noWarningIfEmpty] [$returnThisValueIfNotDefined] [$assignToThisNameInstead]
#
# Reads the value of a parameter ``$name`` in a parameters file
# ``$file`` and assigns it to a variable with the same name ``$name``.
#
# Remark: Same idea as something like ``myvar=$(readFromParamFile
# ...)``, but here the function can make the calling script die if
# there is an error.
#
# * $errMsgPrefix: in case of error, prints this before the error message (used to identify the calling script)
# * $separator: separator used in the parameters file between the name and the value. Default: ``=``.
# * $noWarningIfEmpty: by default a warning is printed to STDERR if the parameter exists but has no value. Set this argument to anything but empty not to print any warning.
# * $returnThisValueIfNotDefined: by default an error is raised and the script is stopped if the parameter is not defined. If this argument is defined, its value is simply returned instead.
# * *assignToThisNameInstead: if defined, ``$name`` is the name read from the file but the value is assigned to the variable ``$assignToThisNameInstead``
#
#/twdoc
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


function absPath {
    d="$1"
    pushd "$d" >/dev/null
    pwd
    popd >/dev/null
}




OPTIND=1
while getopts 'hfn:' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"l" ) trainDir="$OPTARG";;
	"a" ) testDir="$OPTARG";;
	"f" ) forceOpt="-f";;
	"n" ) dataFilePrefix="$OPTARG";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 5 ]; then
    echo "Error: expecting 5 args." 1>&2
    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi
dataDir="$1"
configFile="$2"
templateDir="$3"
refDataDir="$4"
workDir="$5"

if [ ! -f "$dataDir/$dataFilePrefix.parsemetsv" ] || [ ! -f "$dataDir/$dataFilePrefix.conllu" ]; then
    echo "Error: cannot find '$dataDir/$dataFilePrefix.parsemetsv' and/or '$dataDir/$dataFilePrefix.conllu'" 1>&2
    exit 3
fi

if [ ! -f "$configFile" ]; then
    echo "Error: cannot open config file '$configFile'" 1>&2
    exit 2
fi
if [ ! -d "$templateDir" ]; then
    echo "Error: no directory '$templateDir' found" 1>&2
    exit 2
fi
if [ ! -d "$refDataDir" ]; then
    echo "Error: no directory '$refDataDir' found" 1>&2
    exit 2
fi

[ -d "$workDir" ] || mkdir "$workDir"


readFromParamFile "$configFile" "refCorpus"
readFromParamFile "$configFile" "crfTemplate"
readFromParamFile "$configFile" "crfCValue"

templateDirAbs=$(absPath "$templateDir")


outputDoneNb=$(ls "$workDir"/rnd.bio.tst.predict.? 2>/dev/null | wc -l)
if [ ! -z "$forceOpt" ] || [  $outputDoneNb -ne 5 ]; then
    rm -f "$workDir/train.parsemetsv" "$workDir/train.conllu"
    dataDirAbs=$(absPath "$dataDir")
    pushd "$workDir" >/dev/null
    ln -s "$dataDirAbs/$dataFilePrefix.parsemetsv" train.parsemetsv
    ln -s "$dataDirAbs/$dataFilePrefix.conllu" train.conllu

    echo "$progName: running CRF cross-validation"
    cv-prepdata.sh . train 170128.297316 "--bio"  || exit $?
    cv-runexps.sh . "$templateDirAbs/$crfTemplate" "$crfCValue" "-n 10"  || exit $?
    popd >/dev/null
else
    echo "$progName: skipping CRF CV, already done" 1>&2
fi



if [ "$refCorpus" == "NONE" ]; then
    # CRF CV already done
    # output = eval.$n for every fold
    avgeval.pl "$workDir"/eval.? >"$workDir"/cv-eval.out
else

    for n in $(seq 0 4); do

	if [  ! -z "$forceOpt" ] || [ ! -f  "$workDir/$n/output.parsemetsv" ] || [ ! -f  "$workDir/$n/test-eval.out" ]; then
	    echo "$progName: sematic reranker CV fold $n"
	    # concatenating train set from CRF++ predictions
	    for t in $(seq 0 4); do
		if [ $t -ne $n ]; then
		    cat "$workDir/rnd.bio.tst.predict.$t"
		    echo # add empty line
		fi
	    done >"$workDir/$n/rnd.bio.trn.predict.$n"
	    
	    eval "sem-reranker-train-test.sh -l '$workDir/$n/rnd.bio.trn.predict.$n' $forceOpt \"$configFile\" \"$refDataDir\" \"$workDir/$n/weka.model.$n\"  \"$workDir/$n\"" || exit $?
	    eval "sem-reranker-train-test.sh -a \"$workDir/rnd.bio.tst.predict.$n\" $forceOpt \"$configFile\" \"$refDataDir\" \"$workDir/$n/weka.model.$n\"  \"$workDir/$n\"" || exit $?
	    evaluate.py "$workDir/$n/rnd.parsemetsv.tst" "$workDir/$n/output.parsemetsv" >"$workDir/$n/test-eval.out" || exit $?
	else
	    echo "$progName: skipping fold $n, already done" 1>&2
	fi
    done
    avgeval.pl "$workDir"/*/test-eval.out >"$workDir"/cv-eval.out
    
fi
echo "Done, result in $workDir/cv-eval.out"

