#!/bin/bash



progName=$(basename "$BASH_SOURCE")

wekaOutDir=""
cv="yep"

function usage {
  echo
  echo "Usage: $progName [options]  <experiments dir> <expe id> <results file>"
  echo
  echo "  Collects 'test/analysis.tsv' for every language and every fold, and concatenate them"
  echo "  in a single reults file."
  echo
  echo "  - <experiments dir> contains all the languages dirs" 
  echo "  - <expe id> is the name of the experiment to collect" 
  echo
  echo "  Files collected are: <experiments dir>/<lang>/cv/<expe id>/<fold>/test/analysis.tsv"
  echo
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -w <output dir>: also collect 'train/weka.output' files and store them in <output dir>"
  echo "    -1 not CV expe: dir is 'train-test' instead of 'cv' and there are no 'fold' subdirs"
  echo
}



function processFold {
    lang="$1"
    foldDir="$2"
    foldNo="$3"
    printHeader="$4"

    if [ ! -f "$foldDir/test/analysis.tsv" ]; then
	echo "Warning: no file '$foldDir/test/analysis.tsv' found" 1>&2
    else
	if [ ! -z "$printHeader" ]; then # print header only once
	    head -n 1 "$foldDir"/test/analysis.tsv
	fi
	# replace apostrophe with _ (for reading file in R)
	tail -n +2 "$foldDir"/test/analysis.tsv | tr "'" "_" | while read l; do # start at line 2 (after header) 
	    echo -e "$lang\t$foldNo\t$l"
	done
	if [ ! -z "$wekaOutDir"  ]; then
	    cat "$foldDir"/train/weka.output >"$wekaOutDir/$lang.$foldNo.weka.output"
	fi
    fi
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
if [ $# -ne 3 ]; then
    echo "Error: expecting 3 args." 1>&2
    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi


expesDir="$1"
expeId="$2"
output="$3"



echo -ne "language\tfold\t" >"$output"
printHeader=1
for langDir in "$expesDir"/*; do
    if [ -d "$langDir" ]; then
	if [ -z "$cv" ]; then
	    if [ -d "$langDir/train-test/$expeId" ]; then
		lang=${langDir##*/}
		processFold "$lang" "$langDir/train-test/$expeId" 0 "$printHeader"  >>"$output"
		printHeader=""
	    else
		echo "Warning: dir '$langDir/train-test/$expeId' not found" 1>&2
	    fi
	else
	    if [ -d "$langDir"/cv/"$expeId" ]; then
		lang=${langDir##*/}
		#	echo "$lang" 1>&2
		for fold in $(seq 0 4); do 
		    processFold "$lang" "$langDir/cv/$expeId/$fold" "$fold" "$printHeader"  >>"$output"
		    printHeader=""
		done
	    else
		echo "Warning: dir '$langDir/cv/$expeId' not found" 1>&2
	    fi
	fi
    fi
done
