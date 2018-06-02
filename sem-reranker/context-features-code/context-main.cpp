/*
 *
 * context-main.cpp
 *
 */


#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <unordered_map>
#include "Corpus.h"
#include "Context.h"
#include "misc.h"
using namespace std;

string usageStr = "<corpus file> <word1> [<word2> ...]";


int main(int argc, char * argv[]){

  string corpusFile;
  vector<string> words;
   unordered_map<string, string> options = initDefaultOptions();

  // parsing options
  vector<string> mainArgs = parseOptions(argc, argv, options, usageStr, 2, 999);
  
  corpusFile = mainArgs[0];
  int argIndex=1;
  while (argIndex < mainArgs.size()) {
    words.push_back(mainArgs[argIndex]);
    argIndex++;
  }

  printStringStringMap(options);
  cout << " INFO expr = ";
  printStringVector(words);
  cout << endl;

  // build corpus
  Corpus corpus(corpusFile, stoi(options["minFreq"]), stoi(options["removeNMostFreq"]), options["simMeasureId"]);
  Context exprContext(words, stoi(options["halfWindowSize"]), 0, stoi(options["normalizeContext"]), corpus);
  //  exprContext.prntContext(false);
  //  vector<Context> empty; // temp
  vector<double> feats= exprContext.internalFeatures();
  vector<vector<double>> all;
  all.push_back(feats);
  vector<string> names = exprContext.internalFeaturesNames();
  writeArff("test.arff", names, all);
  
}



