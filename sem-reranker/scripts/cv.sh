#!/bin/bash



progName=$(basename "$BASH_SOURCE")

parallelPrefix=
forceOpt=""
cleanupOpt=""

function usage {
  echo
  echo "Usage: $progName [options]  <lgge dir> <config file>"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -f force restart from scratch: remove dir if it exsits"
  echo "    -c clean up: remove directory and keep only performance result"
  echo
}



OPTIND=1
while getopts 'hfc' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) forceOpt="-f";;
	"c" ) cleanupOpt="-c";;
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
configFile="$2"

confId=$(basename "${configFile%.conf}")


[ -d "$mainDir"/cv ] || mkdir "$mainDir"/cv
[[ -d "$mainDir/cv/$confId" ]] || mkdir "$mainDir/cv/$confId"
date >"$mainDir/cv/$confIdtime.start"
for fold in $(seq 0 4); do
    foldDir="$mainDir/cv/$confId/$fold"
    [[ -d  "$foldDir" ]] || mkdir "$foldDir"
    rm -f "$foldDir/eval.out"
    comm="process-train-test.sh \"$mainDir\" \"$foldDir\" \"$configFile\" \"traincv-train.$fold\" \"traincv-test.$fold\" \"traincv-gold.$fold.parseme\""
    eval "$comm"
done


#echo "final CV average '$mainDir'..."
errors=""
for fold in $(seq 0 4); do
    if [ ! -f "$mainDir/cv/$confId"/$fold/eval.out ] || grep ERROR "$mainDir/cv/$confId"/$fold/eval.out >/dev/null; then
	errors="$errors $fold"
    fi
done
nbErrors=$(echo "$errors" | wc -w)
if [ $nbErrors -eq 0 ]; then
    avgeval.pl "$mainDir/cv/$confId"/?/eval.out >"$mainDir/cv/$confId/eval.avg.out"
    if [ ! -z "$cleanupOpt" ]; then
	rm -rf "$mainDir/cv/$confId"/? # removing data, keeping only final score
    fi
else
    echo "Errors happened in '$mainDir/cv/$confId'" 1>&2
    echo "ERROR" >"$mainDir/cv/$confId/eval.avg.out"
    # not removing data
fi
date >"$mainDir/cv/$confIdtime.end"


