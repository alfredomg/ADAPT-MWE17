# temp-mwe17

## Requirements

* [CRF++](https://taku910.github.io/crfpp/) must be installed and accessible via `PATH`
* The shared task data can be downloaded or cloned from https://gitlab.com/parseme/sharedtask-data
* pre-tokenized Europarl data to be used as reference data can be downloaded from TODO

Remark: Weka is included in the repository.

## Installation

From the main directory run:

```
source setup-path.sh
```
This will compile the code if needed and add the relevant directories to `PATH`.

## Usage


## Configuration files

The scripts are meant to be used with configuration files which contain values for the parameters. Examples can be found in directory `conf`. Additionally, a batch of configuration can be generated using:

```

```

## Reference data

* Reference data, including pre-tokenized Europarl corpus for most languages, can be downloaded from **TODO**
* If using the training set as reference data, set `refCorpus=train.conllu` in the config file and specify the input directory containing the shared task data as "reference data dir".


## 

# Details

## semantic-reranker/context-features-code

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
