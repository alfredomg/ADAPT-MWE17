#!/bin/bash


progName=$(basename "$BASH_SOURCE")

force=0
useTestBIOGold=""

function usage {
  echo
  echo "Usage: $progName [options] <lgge dir> <processDir> <config file> <train input file> <test input file> <test gold file>"
  echo
  echo "   <processDir> is either: "
  echo "   - <lggeDir>/cv/confId/foldNo if doing CV"
  echo "   - <lggeDir>/<train-test>/confId/ if doing single full train-test"
  echo "   the directory must exist before"
  echo
  echo "  Options:"
  echo "    -h this help"
  echo "    -f force overwriting previous results"
  echo "    -g special option for using test-gold.bio in prediction analysis. MUST BE USED ONLY WHEN NEEDED."
  echo
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
            echo "Warning: target file $1 does not exist" 1>&2
        fi
        shift
    done
}





function process {
    local mainDir="$1"
    local workDir="$2"
    local configFile="$3"
    local trainInputFile="$4"
    local testInputFile="$5"
    local goldFile="$6"


    # train
    [ -d "$workDir"/train ] || mkdir "$workDir"/train
    linkAbsolutePath "$workDir"/train/input "$mainDir/$trainInputFile"
    linkAbsolutePath "$workDir"/train/reference "$mainDir/$trainInputFile"
    linkAbsolutePath "$workDir"/train/ep.tok "$mainDir/ep.tok"
    eval "apply-config-train-test.sh train \"$workDir/train\" \"$configFile\" \"$workDir/weka.model\""

    # test
    [ -d "$workDir"/test ] || mkdir "$workDir"/test
    linkAbsolutePath "$workDir"/test/input "$mainDir/$testInputFile"
    linkAbsolutePath "$workDir"/test/reference "$mainDir/$testInputFile"
    linkAbsolutePath "$workDir"/test/ep.tok "$mainDir/ep.tok"
    if [ ! -z "$useTestBIOGold" ]; then
	linkAbsolutePath "$workDir"/test/test-gold.bio "$mainDir/test-gold.bio"
    fi
    eval "apply-config-train-test.sh test \"$workDir/test\" \"$configFile\" \"$workDir/weka.model\""


    if [ -s "$workDir/test/output.parseme" ]; then
	evaluate.py "$mainDir/$goldFile" "$workDir/test/output.parseme" >"$workDir/eval.out"
    else
	echo "ERROR"  >"$workDir/eval.out"
    fi
}



OPTIND=1
while getopts 'hfg' option ; do 
    case $option in
	"h" ) usage
 	      exit 0;;
	"f" ) force=1;;
	"g" ) useTestBIOGold="yep";;
 	"?" ) 
	    echo "Error, unknow option." 1>&2
            printHelp=1;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -ne 6 ]; then
    echo "Error: expecting 6 args." 1>&2
    printHelp=1
fi

if [ ! -z "$printHelp" ]; then
    usage 1>&2
    exit 1
fi
mainDir="$1"
processDir="$2"
configFile="$3"
trainFile="$4"
testFile="$5"
goldFile="$6"

confId=$(basename "${configFile%.conf}")
if [ $force -eq 1 ]; then
    rm -rf "$processDir"
    mkdir "$processDir"
fi
process "$mainDir"/input "$processDir" "$configFile" "$trainFile" "$testFile" "$goldFile"
cleanup-expe-dir.sh "$processDir"
