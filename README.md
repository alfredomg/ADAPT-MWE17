# ADAPT CENTRE participation to the shared task on Verbal Multiword Expressions 2017 (VMWE17)

*Authors:  Alfredo Maldonado, Lifeng Han, Erwan Moreau, Ashjan Alsulaimani, Koel Dutta Chowdhury, Carl Vogel and Qun Liu.*


## Links

* [Shared task website](http://multiword.sourceforge.net/PHITE.php?sitesig=CONF&page=CONF_05_MWE_2017___lb__EACL__rb__)
* [Shared Task data (gitlab)](https://gitlab.com/parseme/sharedtask-data)
* [Our paper](https://aclanthology.coli.uni-saarland.de/papers/W17-1715/w17-1715)
* [Reference data](https://drive.google.com/file/d/1ConYT3A8JtRmrmnWZBxSALUMMhMphG7o/view?usp=sharing) (1.8 GB) (remark: it's normal that the preview doesn't work, simply click on the download link)
  * For most of the VMWE17 languages, contains several version of Europarl which can be used as "reference corpus" with the semantic reranker.
  * Uncompress with `tar xfj reference-data.tar.bz2`

## Requirements

* [CRF++](https://taku910.github.io/crfpp/) must be installed and accessible via `PATH`
* The shared task data can be downloaded or cloned from https://gitlab.com/parseme/sharedtask-data
* pre-tokenized Europarl data to be used as reference data can be downloaded from the link above

Remark: Weka is included in the repository.

## Installation

From the main directory run:

```
source setup-path.sh
```

This will compile the code if needed and add the relevant directories to `PATH`. You can add this to your `.bashrc` file in order to have the `PATH` set up whenever you open a new session.

## Usage


### Simple training + testing

From the main directory:

```
run-config-train-test.sh -l sharedtask-data/1.0/FR -a sharedtask-data/1.0/FR conf/default.conf crf/templates/ reference-data/FR/ model output
```

* `-l` (learn) for training using the training data provided in the directory
* `-a` (apply) for testing using the test data provided in the directory
* using configuration file `conf/default.conf`
* `reference-data` contains the reference data (see link above)
* `model` will contain the model at the end of the process
* `output` is the "work directory"; at the end of the testing process it contains:
  * The predictions in `parsemetsv` format are stored in `<work dir>/output.parsemetsv`
  * Evaluation results are stored in `<work dir>/test-eval.out`.


### Reproducing the results from the [paper](https://aclanthology.coli.uni-saarland.de/papers/W17-1715/w17-1715)

Requires the reference data downloaded from the link given at the begining of this document.
From the main directory:

```
# Generate the set of config files
mkdir -p experiments/configs; echo conf/vary-templates-and-refcorpus.conf | expand-multi-config.pl -p experiments/configs/ >configs.list
# Generate the tasks to run
generate-tasks.sh -c sharedtask-data/1.0/ configs.list crf/templates/ reference-data/ experiments/cv >tasks
# split to run 10 processes in parallel
split -d -l 18 tasks batch.
# run 
for n in $(seq 0 9); do (bash batch.0$n &); done
```

* See "Configuration files" below for explanations about the generation of the config files with `expand-multi-config.pl`.
* `generate-tasks.sh` creates directories and writes the list of commands to run.
* The last two lines show a simple way to split the processes into 10 batches to run in parallel; the full process can take around 24 hours.
* Some of the configurations will not work with some of the datasets (conllu data or Europarl data not available; see readme in the reference data archive - link at the beginning of this document).
* The results from a large experiment with multiple config files like the above can be collected conveniently as a TSV table: `ls experiments/cv/*/*/cv-eval.out | collect-results.pl experiments/configs/ results.tsv`

## Details

### Configuration files

The scripts are meant to be used with configuration files which contain values for the parameters. Examples can be found in the directory `conf`. Additionally, a batch of configuration files can be generated using e.g.:

```
# caution: generates almost 2 millions config files
mkdir configs; echo conf/options.multi-conf | expand-multi-config.pl configs/ 
```

In order to generate a different set of configurations, either customize the values that a parameter can take in `conf/options.multi-conf` or use the `-r` option to generate a random subset of config files, e.g.:

```
# generate a random 50 config files
mkdir configs; echo conf/options.multi-conf | expand-multi-config.pl -r 50 configs/ 
```



### Reference data

* Reference data, including pre-tokenized Europarl corpus for most languages, can be downloaded from the link given at the start of this document.
* In order to use the training set as reference data, set `refCorpus=train.conllu` in the config file and specify the input directory containing the shared task data as "reference data dir".
  * Remark: `refCorpus=train.conllu:lemma` will make the system use the lemma column instead of the token column.
* In order to use only the CRF component (no semantic reranker), set `refCorpus=NONE` in the config file.



### semantic-reranker/context-features-code

This is the C++ code which extracts the features for the semantic reranking. The semantic reranker consists of two main steps: feature extraction and supervised learning. The former produces features for each of the 10 candidate labelings provided by the CRF component for every sentence. The latter trains or applies a Weka model using said features; the model is meant to score every candidate, so that the highest score can be selected as the best prediction.

Remark: this behaviour means that the reranker has to be trained on (top 10) data predicted by the CRF++ component. Therefore the training stage requires to cross-validation by CRF++, in order to obtain realistic top 10 predictions on the training set.

* Training
  * `context-features-train` produces the features, together with the gold label for every candidate labeling;
  * Weka is used to train a supervised regression model based on these features.
* Testing
  * `context-features-test` produces the features for every candidate labeling;
  * Weka predicts a score based on these features for every candidate labeling;
  * For every sentence, the candidate which obtains the highest score among the 10 possible labelings is selected.

An additional executable `single-expr-features` can be used to generate a subset of the features (called internal features) for a single expression given as a parameter. This executable is used only for debugging or inspection purposes. Example:

```
single-expr-features reference-data/FR/europarl-v7.generic.tok rendre compte
```
