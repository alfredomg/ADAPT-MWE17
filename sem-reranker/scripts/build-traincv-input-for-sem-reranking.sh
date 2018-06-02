#!/bin/bash

progName=$(basename "$BASH_SOURCE")

inputCV10DirName="cv-2017-01-29-correcttemplates-top10"
testsetSubmitName="closed-top10"
goldCVParsemeName="depbi"
europarlName="ep"
sharedTaskGitRepoDir="sharedtask-data"

function usage {
  echo
  echo "Usage: $progName [options]  <data dir> <target dir>"
  echo
  echo "  Assembles the input data for running the semantic reranking part on the"
  echo "  training set with cross-validation. Requires the following subdirectories"
  echo "  under <data dir>:"
  echo 
  echo "    - '$goldCVParsemeName' which contains 'train.rnd.parsemetsv.<test>' for each language;"
  echo "      this file is used as gold standard for evaluation of CV."
  echo "    - '$inputCV10DirName' which contains for each language"
  echo "      a directory (symlink actually) 'selected', which contains "
  echo "      'train.bio.tst.predict.<test>': the output from CRF++ with the 10 most"
  echo "      likely predicted sequences, already split for for cross-validation."
  echo "    - '$testsetSubmitName' which contains the official closed track "
  echo "      submission (test data) as well as the top 10 predictions by CRF++"
  echo "      for the test data."
  echo "    - '$europarlName' which contains europarl-v7.<lang>-en.<lang>.tok for as many languages"
  echo "      as possible (a few languages are not available)."
  echo
  echo "  CAUTION: the generated data requires about 7GB."
  echo
  echo "  Options:"
  echo "    -h this help"
  echo
}


function checkDir {
    if [ ! -d "$1" ]; then
	echo "Error: directory '$1' not found."  1>&2
	exit 2
    fi
}


#twdoc linkAbsolutePath $destEntry $targetFile1 [... $targetFileN]
#
# * $destEntry is a file or directory where the symbolic link(s) will be created:
# ** if $destEntry is a directory, creates a symlink located in $destEntry pointing to $targetFileI for each file $targetFileI;
# ** if $destEntry is a file, creates a symlink $destEntry pointing to $targetFile1 (only one target file permitted).
#
#/twdoc
function linkAbsolutePath {
    local thisDir=$(pwd)
    local dest="$1"
    local targetDir=
    local dir=
    shift
    while [ ! -z "$1" ]; do # for every target file
        if [ -e "$1" ]; then
            cd $(dirname "$1")
            targetDir=$(pwd)
            cd "$thisDir"
            if [ -d "$dest" ]; then
                cd "$dest"
                if [ -e "$(basename "$1")" ]; then
                    echo "Warning: '$(basename "$1")' already exists in '$dest'" 1>&2
                else
                    ln -s "$targetDir/$(basename "$1")"
                fi
            else
                dir=$(dirname "$dest")
                if [ -e "$dir" ]; then
                    cd "$dir"
                    if [ -e $(basename "$dest") ]; then
                        echo "Warning: '$(basename "$dest")' already exists in '$dir'" 1>&2
                    else
                        ln -s "$targetDir/$(basename "$1")" "$(basename "$dest")"
                    fi
                else
                    echo "Error: dest dir $dir does not exist" 1>&2
                    exit
                fi
            fi
            cd "$thisDir"
        else
            echo "Error: target file $1 does not exist" 1>&2
            exit
        fi
        shift
    done
}





OPTIND=1
while getopts 'h' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
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
dataDir="$1"
targetDir="$2"


checkDir "$dataDir"
checkDir "$dataDir/$goldCVParsemeName"
checkDir "$dataDir/$inputCV10DirName"
checkDir "$dataDir/$testsetSubmitName"
checkDir "$dataDir/$europarlName"

[ -d "$targetDir" ] || mkdir "$targetDir"

for langDir in "$dataDir/$inputCV10DirName"/*; do
    if [ -d "$langDir" ]; then
	lang=$(basename "$langDir")
	echo -n "$lang;  "
	[ -d "$targetDir/$lang" ] || mkdir "$targetDir/$lang"
	[ -d "$targetDir/$lang/input" ] || mkdir "$targetDir/$lang/input"
	for n in $(seq 0 4); do
	    linkAbsolutePath "$targetDir/$lang/input/traincv-test.$n" "$langDir/selected/train.bio.tst.predict.$n"
	    linkAbsolutePath "$targetDir/$lang/input/traincv-gold.$n.parseme" "$dataDir/$goldCVParsemeName/$lang/train.rnd.parsemetsv.$n"
	done
	rm -f "$targetDir/$lang/input/train-full"
	for testsetNo in $(seq 0 4); do # train set CV: concat other test sets
	    cat "$targetDir/$lang/input/traincv-test.$testsetNo" >>"$targetDir/$lang/input/train-full"
	    rm -f "$targetDir/$lang/input/traincv-train.$testsetNo"
	    for n in $(seq 0 4); do
		if [ $n -ne $testsetNo ]; then
		    cat "$targetDir/$lang/input/traincv-test.$n" >>"$targetDir/$lang/input/traincv-train.$testsetNo"
		fi
	    done 
	done
	langLC=$(echo "$lang" | tr '[:upper:]' '[:lower:]')
	if [ -f "$dataDir/$europarlName/europarl-v7.$langLC-en.$langLC.tok" ]; then
	    linkAbsolutePath "$targetDir/$lang/input/ep.tok" "$dataDir/$europarlName/europarl-v7.$langLC-en.$langLC.tok" 
	else
	    echo "Warning: no Europarl corpus for language '$lang'" 1>&2
	fi
	linkAbsolutePath "$targetDir/$lang/input/testset" "$dataDir/$testsetSubmitName/$lang/test.system.bio" 
	# added later:
	linkAbsolutePath "$targetDir/$lang/input/testset-gold.parseme" "$dataDir/$sharedTaskGitRepoDir/$lang/test.parsemetsv"
    fi
done
echo

