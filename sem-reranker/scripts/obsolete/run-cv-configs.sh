#!/bin/bash



progName=$(basename "$BASH_SOURCE")

parallelPrefix=
forceOpt=""
cleanupOpt=""
cv="yes"

function usage {
  echo
  echo "Usage: $progName [options] <lgge dir> <configs list>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -f force restart from scratch: remove dir if it exsits"
  echo "    -c clean up: remove data and keep only performance result"
  echo "    -P <parallel prefix>"
  echo "    -1 full train test, no CV"
  echo
}




OPTIND=1
while getopts 'hP:fc1' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) forceOpt="-f";;
	"c" ) cleanupOpt="-c";;
	"P") parallelPrefix="$OPTARG";;
	"1" ) cv="";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 2 ]; then
    echo "Error: expecting 2 args." 1>&2
    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi
mainDir="$1"
configsListFile="$2"


cat "$configsListFile" | while read configFile; do

    confId=$(basename "${configFile%.conf}")
    if [ -z "$cv" ]; then
	[ -d "$mainDir"/train-test ] || mkdir "$mainDir"/train-test
	[[ -d "$mainDir/train-test/$confId" ]] || mkdir "$mainDir/train-test/$confId"
	resFile="$mainDir/train-test/$confId/eval.out"
    else
	[ -d "$mainDir"/cv ] || mkdir "$mainDir"/cv
	[[ -d "$mainDir/cv/$confId" ]] || mkdir "$mainDir/cv/$confId"
	resFile="$mainDir/cv/$confId/eval.avg.out"
    fi
    if [ ! -z "$forceOpt" ] || [ ! -s "$resFile" ] || grep ERROR $resFile>/dev/null; then
	rm -f "$resFile"
	if [ -z "$cv" ]; then
	    # careful, the -g option is a dirty trick only for real test set for analysis
	    comm="process-train-test.sh -g $forceOpt \"$mainDir\" \"$mainDir/train-test/$confId\" \"$configFile\" train-full testset testset-gold.parseme >$mainDir/train-test/$confId.out 2>$mainDir/train-test/$confId.err"
	else
	    comm="cv.sh $cleanupOpt $forceOpt \"$mainDir\" \"$configFile\" >$mainDir/cv/$confId.out 2>$mainDir/cv/$confId.err"
	fi
	if [ -z "$parallelPrefix" ]; then
	    eval "$comm"
	else
	    taskFile=$(mktemp "$parallelPrefix.$confId.XXXXXXXXX")
	    echo "$comm" >"$taskFile"
	fi
    else
	echo "config $confId already computed for language $(basename $mainDir), skipping"
    fi
done


