
#ifndef _CONTEXT_H_
#define _CONTEXT_H_ 

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Corpus.h"
#include "Sentence.h"
//#include "ContextCollection.h"

using namespace std;

// this is a "forward declaration": the compiler needs to know that this class exists in order accept the pointers inside the class,
// but doesn't need the content.
// the real class is included in the cpp file
class ContextCollection;

class Context { // this class is for one expression only!
 public:
  Context(vector<string> words, int halfWindowSize, int maxDist, int normalizeByExprOccurrence, bool windowIncludeWordsINexpr, bool windowMultipleOccurrencesPosition,Corpus *corpusIn);

  // there is only one corpus so its good to use reference to it in each object context.
  string getExprAsString();
  void addToContext(int sentenceId);
  void addToContext(int sentenceId, vector<int> &positions);
  int getFrequency();


  double simJaccard(Context &context2);
  double simMinMax(Context &context2);
  double simCosine(Context& context2, bool withIDF);
  void prntContext(bool prntSentences);
  // removed version with no parameter: new Context objects for pseudo-expressions should always be added to the collection
  //void computeInternalFeatures();
  void computeInternalFeatures(ContextCollection *contextColl, string featGroups, string featTypes, string featStd, bool featOnlyMean);
  vector<double> externalFeatures(vector<Context *> otherPossibleContexts, string featTypes, string featStd, bool featOnlyMean);
  // returns true if the features have been already computed for this object
  int hasInternalFeatures();
  vector<double> &getInternalFeatures();
  vector<double> featuresComparison(vector<Context *> probe, string featTypes, bool featOnlyMean);
  vector<double> stats(vector<double> values);
  double computeSimScore(Context &context2);

 private:
  vector<string> expr; // it contains one single expression.
  Corpus *corpus; // there is only one corpus so its good to use pointer to it in each object context // update: used to be a reference but now we need to be able to assign NULL to it

  // the actual context vector, contains frequencies of words found the context window
  unordered_map<string, double> cooc; // one expression
  int halfWindowSize;
  //halfWindowSize means the num words taken from the left and the num of words taken from the right of the expression.  Windowsize then is the sum of these.
  //* e.g. if halfwindow is 2 both sides then the windowsisze is 4.
  int maxDist; // how far away the position of the vocab of the expression in a sentence.

  // frequency of the expression in the reference corpus
  int freq;
  int normalizeByExprOccurrence;// the frequency of the word (either from the left part or the right part) in a sentence
  vector<double> internalFeatures;
  
  /*these options will lets choose between either to include the expression words or exclude them inside the vecto(window)?
   * or also either to include multiple occurrences of the words or only including single occurrence of the word(position)
  */
  bool windowIncludeWordsINexpr;
  bool windowMultipleOccurrencesPosition;

  void addWordsToContext(vector<string> &words);
  void buildFromCorpus();
};

// non class functions:
vector<double> zeroLengthInternalFeatures(string featGroups, string featTypes, string featStd, bool featOnlyMean);
vector<double> zeroLengthInternalFeatures(int length, double exprFreq, string featGroups, string featTypes, string featStd, bool featOnlyMean);
vector<double> zeroLengthExternalFeatures(string featTypes, string featStd, bool featOnlyMean);
vector<string> internalFeaturesNames(string featGroups, string featTypes, string featStd, bool featOnlyMean);
vector<string> externalFeaturesNames(string featTypes, string featStd, bool featOnlyMean);
#endif

/*NormalizingByExpr0ccurrence:
*1-one case we have propotion to each word. we give big weight to each word if the windowsize is small.
*2- we give big wieght to the whole group together if we have a big windowsize.
*
*NormalizedByExpr0occ can go either zero or one and can work for one expression in this sentnece and for that in the whole corpus:
*(FOR ONE SENTENCE S):
*1-ZERO CASE:
*the frequency of the word  (within the window size i.e. the context) **in this sentence** that contains the expression [#w^s]
*2-ONE CASE:
*the frequency of the word  (within the window size) in the sentence that contains the expression [#w^s] / the summation over the frequency of words in the context in **this sentence** [sum w #w^s]
*______________________________________________________________________________________________________________________________________________________________________________
*(FOR THE WHOLE CORPUS)
*1-ZERO CASE:
*int frequency of the word in the whole corpus [sum s^i #w^si] / the frequency of sentences which contain the exprssion in the corpus[sum si 1]
*2- ONE CASE:
*the summation over s^i [(~the frequency of the word  (within the window size) in the sentence that contains the expression [#w^s] / the summation over the frequency of words in the context in **this sentence** [sum w #w^s]~)]   /  (~ the number of sentneces which contain the expression ~).
*
*/



