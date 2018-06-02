

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Context.h"
#include "annotatedcorpus.h"
#include "misc.h"

using namespace std;

class ContextCollection {

 public:
  // All the candidate expressions will be added to the collection, and immediately the "internal features" are computed;
  // in Context::computeInternalFeatures the sub-expressions are created and added to the collection through addContext, but
  // their features are not computed (since we don't need them).
  // In case a sub-expression turns out to also be a candidate expression, its feature are calculated at this time.
  //
  // if corpusIn is NULL then the context vectors are NOT computed
  ContextCollection(annotatedcorpus &data, unordered_map<string, string> options, Corpus *corpusIn, bool training);
  void addAllExprsInMap(outcomes *o);
  Context *findInColl(vector<string> expr);
  Context *findInColl(string exprAsString);
  void addContext(Context *c);
  void printExpressions(string filename);
  int size();
  
 private:
  double minConfidence; // if the confidence is too low we ignore it.
  unordered_map<string, Context *> exprs;
  int halfWindowSize;// inistialise the context object
  int maxDist;
  int normalizeByExprOccurrence;// inisiliase the context object
  Corpus *corpus;
  annotatedcorpus &annData; // annotated corpus object.
  bool trainingMode;

  // otions for how to compute context window
  bool windowIncludeWordsINexpr;
  bool windowMultipleOccurrencesPosition;

  // options for which features to include
  string featGroups;
  string featTypes;
  string featStd;
  bool featOnlyMean;

};


