#!/bin/bash

progName=$(basename "$BASH_SOURCE")

dir="$HOME/MWE17SharedTask"
forceOpt=""
cleanupOpt=""
resume=0



function usage {
  echo
  echo "Usage: $progName [options] <mode> <main dir> <configs list file> <nb tasks/batch>"
  echo
  echo "  runs process <mode> in parallel with data in <main dir> and for all"
  echo "  the configs in <configs list file>."
  echo
  echo "  <mode> is either: "
  echo "  * 'trainingset-cv' "
  echo "  * 'traintest' "
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -f force restart from scratch: remove dir if it exsits"
  echo "    -c clean up: remove data and keep only performance result (only for 'trainingset-cv')"
  echo "    -r resume mode: tasks are supposed to be already in the tasks dir,"
  echo "       only the 'task distrib daemon' is restarted. Arguments are"
  echo "       ignored."
  echo
}



function absPathDir {
    d="$1"
    if [ -d "$d" ]; then
	pushd "$d" >/dev/null
	pwd
	popd >/dev/null
    else
	echo "Error: no dir '$d'" 1>&2
	exit 3
    fi
}


function trainsetCVAllDatasets {
    mainDir="$1"
    configsList="$2"
    parallelPrefix="$3"
    for datasetDir in "$mainDir"/*; do
	if [ -d "$datasetDir" ]; then
            lang=$(basename "$datasetDir")
            comm="run-cv-configs.sh $cleanupOpt $forceOpt -P $parallelPrefix.$lang  \"$datasetDir\" \"$configsList\" >\"$datasetDir.out\" 2>\"$datasetDir.err\""
	    eval "$comm" &
        fi
    done
}



function fullTrainTestAllDatasets {
    mainDir="$1"
    configsList="$2"
    parallelPrefix="$3"
    for datasetDir in "$mainDir"/*; do
	if [ -d "$datasetDir" ]; then
            lang=$(basename "$datasetDir")
            comm="run-cv-configs.sh -1 $cleanupOpt $forceOpt -P $parallelPrefix.$lang  \"$datasetDir\" \"$configsList\" >\"$datasetDir.out\" 2>\"$datasetDir.err\""
	    echo "$comm" 
        fi
    done
}





OPTIND=1
while getopts 'hfrc' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) forceOpt="-f";;
	"c" ) cleanupOpt="-c";;
	"r" ) resume=1;;
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

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi
mode="$1"
mainDir="$2"
configsList="$3"
nbSlots="$4"

source $dir/semantic-reranker/setup-path.sh

if [ $resume -ne 1 ]; then
    rm -rf $dir/tasks $dir/slurm
    mkdir $dir/tasks
    mkdir $dir/slurm

    mainDir=$(absPathDir "$mainDir")
    configsDir=$(dirname "$configsList")
    configsDir=$(absPathDir "$configsDir")
    configsList="$configsDir"/$(basename "$configsList")

    if [ "$mode" == "trainingset-cv" ]; then
	trainsetCVAllDatasets "$mainDir" "$configsList" $dir/tasks/traincv
    elif [ "$mode" == "traintest" ]; then
	fullTrainTestAllDatasets "$mainDir" "$configsList" $dir/tasks/traintest
    else
	echo "Invalid id '$mode'" 1>&2
	exit 2
    fi
fi



## Remark: the program can require a large amount of memory and all the nodes don't have the same amount afaik
## Jobs which use too much memory are killed, so it makes it hard to see what happens
pushd $dir/slurm >/dev/null
if [ $resume -ne 1 ]; then
    task-distrib-daemon.sh -p 5 -b $nbSlots -i "source $dir/setup-path.sh" -e "task-distrib-script-slurm.sh -n $nbSlots $dir/slurm/ DUMMY compute 1-00:00:00" $dir/tasks 2000
else
    task-distrib-daemon.sh -c -p 5 -b $nbSlots -i "source $dir/setup-path.sh" -e "task-distrib-script-slurm.sh -n $nbSlots $dir/slurm/ DUMMY compute 1-00:00:00" $dir/tasks 2000
fi
popd >/dev/null



