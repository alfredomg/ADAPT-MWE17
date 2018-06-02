#!/bin/bash



progName=$(basename "$BASH_SOURCE")

wekaOutDir=""
cv="yep"


function usage {
  echo
  echo "Usage: $progName [options]  <experiments dir> <results file>"
  echo
  echo "  Reads a list of configs from STDIN, applies collect-analysis.sh to every config and"
  echo "  concatenates the output in <results file>."
  echo "  The configs read as input can be a full path config file which ends with .conf, "
  echo "  the config name will be extracted ( convenient for using a config list file)"
  echo "  Requires 'test/analysis.tsv' to have been computed by apply-predict.pl for every"
  echo "  language/config"
  echo
  echo "  - <experiments dir> contains all the languages dirs" 
  echo
  echo "  Files collected are: <experiments dir>/<lang>/cv/<expe id>/<fold>/test/analysis.tsv"
  echo "  unless option -1 is used: <experiments dir>/<lang>/<expe id>/test/analysis.tsv"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -1 not CV expe: dir is 'train-test' instead of 'cv' and there are no 'fold' subdirs"
  echo
}






OPTIND=1
while getopts 'hw:1' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"w" ) wekaOutDir="$OPTARG";;
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


expesDir="$1"
output="$2"

echo -en "config\t" >"$output"
header=1
while read config; do
    config=$(basename "${config%.conf}")
    outTmp=$(mktemp)
    if [ -z "$cv" ]; then
	collect-analysis.sh -1 "$expesDir" "$config" "$outTmp"
    else
	collect-analysis.sh "$expesDir" "$config" "$outTmp"
    fi
    if [ ! -z "$header" ]; then
	head -n 1 $outTmp 
	header=""
    fi
    tail -n +2 "$outTmp" | while read line; do
	echo -e "$config\t$line"
    done
    rm -f $outTmp
done >>"$output"
