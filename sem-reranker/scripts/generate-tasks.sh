#!/bin/bash


progName=$(basename "$BASH_SOURCE")

forceOpt=""
cv=""

function usage {
  echo
  echo "Usage: $progName [options] <data dir> <configs list file> <template dir> <ref data dir> <output dir>"
  echo
  echo "  Prints the commands to run in order to train, test and evaluate every language"
  echo "  dataset in <data dir> with every config file in <configs list file>."
  echo
  echo "  <data dir> contains language directories, each containing files train.parsemetsv,"
  echo "  train.conllu, test.parsemetsv and test.conllu."
  echo "  <configs list file> is a file containing the path to a config file on every line."
  echo "  <ref data dir> contains language directories, with the corresponding reference"
  echo "  data files; see sem-reranker-train-test.sh for more details."
  echo "  See run-config-train-test.sh for explanations about the parameter <template dir>."
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -c cross-validation mode using only the training file (longer)."
  echo "    -f force recomputing even if output already there."
#  echo "     -n <filename prefix> to use instead of 'train'."
  echo
}






OPTIND=1
while getopts 'hcf' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"c" ) cv="yep";;
	"f" ) forceOpt="-f";;
#	"n" ) dataFilePrefix="$OPTARG";;
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
configsFile="$2"
templateDir="$3"
refDataDir="$4"
workDir="$5"

if [ ! -f "$configsFile" ]; then
    echo "Error: cannot open configs file list '$configsFile'" 1>&2
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


for langDir in "$dataDir"/*; do
    if [ -d "$langDir" ]; then
	if [ -f "$langDir/train.parsemetsv" ] && [ -f "$langDir/train.conllu" ]; then
	    lang=$(basename "$langDir")
	    [ -d "$workDir/$lang" ] || mkdir "$workDir/$lang"
	    cat "$configsFile" | while read config; do
		confId=$(basename ${config%.conf})
		expePath="$workDir/$lang/$confId"
		if [ -z "$cv" ]; then
		    [ -d "$expePath.model" ] || mkdir "$expePath.model" 
		    [ -d "$expePath.output" ] || mkdir "$expePath.output" 
		    echo "run-config-train-test.sh $forceOpt -l \"$langDir\" -a \"$langDir\" \"$config\" \"$templateDir\" \"$refDataDir/$lang\" \"$expePath.model\" \"$expePath.output\" >\"$expePath.out\" 2>\"$expePath.err\""
		else
		    [ -d "$expePath" ] || mkdir "$expePath"
		    echo "run-config-cv.sh  $forceOpt \"$langDir\" \"$config\" \"$templateDir\" \"$refDataDir/$lang\" \"$expePath\" >\"$expePath.out\" 2>\"$expePath.err\""
		fi
	    done
	else
	    echo "Warning: $progName: no file train.parsemetsv and/or train.conllu in $langDir, skipping" 1>&2
	fi
    fi
done
