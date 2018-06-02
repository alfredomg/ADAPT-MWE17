#!/usr/bin/env bash

# Prepares language data for a 5-fold cross validation experiment
# Folder structure is created in current directory. So it is recommended to run in new, empty directory. 
# Usage:
#	cv-prepdata.sh langDir [filename prefix] [SEED] [parseme2bio-Additional-Options]
# Example:
#	cv-prepdata.sh ~/code/repos/sharedtask-data/FR

# added Erwan March 18: filename prefix arg

datadir=$1
filePrefix=$2
seed=$3
p2bopts=$4

if [ -z "$filePrefix" ]; then
    file_ptsv="train.parsemetsv"
    file_conllu="train.conllu"
    echo "No prefix supplied, using default 'train'"
else
    file_ptsv="$filePrefix.parsemetsv"
    file_conllu="$filePrefix.conllu"
fi


if [ ! -z $seed ] ; then 
    echo "Using seed: $seed"
    seed="--seed $seed"
else
    echo "No seed set. Internal random initialisation."
fi

# Randomise:

conll="$datadir/$file_conllu"
if [ -f "$conll" ] ; then
    conll="--conll $conll"
    conllrnd="--conll rnd.conllu"
else
    conll=""
    conllrnd=""
fi
randomise.pl --ptsv "$datadir/$file_ptsv" $conll $seed --outpref rnd || exit $?


# Create 5 folds for CV:

# split gold standard (for evaluation purposes):
split.pl --input rnd.parsemetsv --split "0.2,0.2,0.2,0.2,0.2" || exit $?

# merge parsemetsv and conllu into bio format and split int 5 folds:
parseme2bio.pl --ptsv rnd.parsemetsv $conllrnd $p2bopts --output rnd.bio || exit $?
split.pl --input rnd.bio --split "0.2,0.2,0.2,0.2,0.2" || exit $?

# create subdirectories with folds: 
for i in {0..4} 
do
    echo "Fold $i:"
    if [ ! -d $i ] ; then
        mkdir $i || exit $?
    fi
    bio_trn=""
    gol_tst=""
    bio_tst=""
    for j in {0..4}
    do
        if [ $i == $j ] ; then # j is test
            gol_tst="rnd.parsemetsv.$j"
            bio_tst="rnd.bio.$j"
        else # j is part of training
            bio_trn="$bio_trn rnd.bio.$j"
        fi
    done
    echo "  train:$bio_trn"
    echo "  test:  $bio_tst"
    echo "  gold:  $gol_tst"
    cat $bio_trn > "$i/rnd.bio.trn" || exit $?
    cp $gol_tst "$i/rnd.parsemetsv.tst" || exit $?
    cp $bio_tst "$i/rnd.bio.tst" || exit $?
done


