

#include <algorithm>
#include <unordered_map>
#include <math.h>
#include <limits>
#include "Context.h"
#include "misc.h"
#include "ContextCollection.h"

using namespace std;



/*
 * if halfWindowSize is zero, the context is taken to be the whole sentence.
 * minFreq and maxFreq are thresholds about the context words to take into account.
 * maxDist is the max length between the first and the last word in the expression (0 means no max).
 *
 */


/* halfWindowSize means the num words taken from the left and the num of words taken from the right of the expression.  Windowsize then is the sum of these.
 * e.g. if halfwindow is 2 both sides then the windowsisze is 4.
 */

//vector<string>words is the insialation of vector<string>expr.// ITS EXPR (WORDS) AN ASSIGNMENT
Context::Context(vector<string> words, int halfWindowSize, int maxDist, int normalizeByExprOccurrence, bool  windowIncludeWordsINexpr, bool windowMultipleOccurrencesPosition,
		Corpus *corpusIn) : corpus(corpusIn), expr(words) {
	//  cerr << "debug: new Context for ";
	//printStringVector(expr);
	//cerr << endl;
	if (expr.size() == 0) {
		cerr << "Error: expression doesn't contain any word\n";
		exit(1);
	}

	//we are insilaising our context object to a certain expression
	this->halfWindowSize = halfWindowSize;
	this->maxDist = maxDist;
	this->normalizeByExprOccurrence = normalizeByExprOccurrence;
	cooc.clear();
	freq = 0;

	this -> windowIncludeWordsINexpr = windowIncludeWordsINexpr;
	this -> windowMultipleOccurrencesPosition = windowMultipleOccurrencesPosition;


	buildFromCorpus();// to look inside europal
}

// context object will contains the full single expression in the member vector<string> expr.
string Context::getExprAsString() {
	return joinStrings(expr, " ");
}

//look for the one expression in the REFERENCE corpus. AND FILL COOC
void Context::buildFromCorpus() {
	if (corpus != NULL) { //TRYING TO FIND THE EXPRESSIOMN FRTOM CRF OR ANY WHERE COMPUTE INTERANAL IN THE REFERENCE CORPUS SENTENCES
		unordered_set<int> sentencesContainingExpr = corpus->sentencesWhichContain(expr);// we want the sentences which contains the expression
		for (int sentNo : sentencesContainingExpr) {// now we selected the sentences that we want for this expression now we should do the window. loop deal with one senteces.
			addToContext(sentNo);
		}

		// normalizing wrt number of sentences
		int nbSent = getFrequency();
		for ( auto it = cooc.begin(); it != cooc.end(); ++it){
			it->second /= nbSent;
		}
	} else {
		cerr << "Warning: corpus is NULL, not computing context vectors for candidate expression '"<< joinStrings(expr," ")<<"' "<<endl;
	}

}



int Context::getFrequency() {
	return freq;
}


void Context::addToContext(int sentenceId) {// SENTENCEID is from the reference corpus Europal

	if (corpus != NULL) {
		// extract positions
		Sentence &s = corpus->getSentence(sentenceId);
		vector<int> pos = s.findExpression(expr, maxDist);  //maxDist?


		addToContext(sentenceId, pos);

	} else {
		cerr << "Bug: corpus must not be NULL here (addToContext(int))\n";
		exit(7);
	}
}



/* This function addToContext has to deal with many options depending on the command line choices:
 *
 * for (int i=0; i<positions.size(); i++) { // remark: in multi words expr the other words of the expr might be included (possibly even several times)
				//			cerr << "i="<< i << endl;
				if (halfWindowSize>0) {

					for (int windowPos=positions[i]-halfWindowSize; (windowPos<s.getwords().size()) && (windowPos<=positions[i]+halfWindowSize); windowPos++) {
						if ((windowPos>=0) && (windowPos!=positions[i])) {// windowPos<o segmentation fault.

							int currentWordIsNOTInExpr = find(positions.begin(), positions.end(), windowPos) == positions.end();
							if (windowIncludeWordsINexpr || currentWordIsNOTInExpr) {
							//if (windowIncludeWordsINexpr || (find(positions.begin(), positions.end(), windowPos) == positions.end())) {
								if (corpus->isContextVocabWord(words[windowPos])) {
									//							cerr << "L word="<< words[l] << endl;
									if (windowMultipleOccurrencesPosition) {
										contextVector.push_back(words[windowPos]);
									} else {
										contextSet.insert(words[windowPos]);
									}
								}
							}
						}
					}
					// I'm not sure that we really need this case (no window size, take all the words in the sentence)
				} else { // if window size = 0, take all the words, except the ones in expr
					if (find(positions.begin(), positions.end(), i) == positions.end()) { // if i is not one of the indexes of the expression
						if (corpus->isContextVocabWord(words[i])) {
							if (windowMultipleOccurrencesPosition) {
								contextVector.push_back(words[i]);
							} else {
								contextSet.insert(words[i]);
							}
						}
					}
				}
			}
 *
 *
 *
 *Example:
 *
 *positions < 2, 3, 6>             " i is the first for loop for positions in the Expression"
 *
 *words < I am cat and he is dog>  " windowPos is the second for loop in the Sentence"

 *
 *
 * Option one with Windowscope:
 *
 *
 * if ((windowPos>=0) && (windowPos!=positions[i])) {
 *
 *all these 4 cases happen when the repetitions fall within the window scope:
 *
 *
 *
 * 1-currentWordIsNOTInExpr and NOTwindowMultipleOccurrencesPosition
 *  This is the default option if the user don't specify any preference (0,0).
 *  In this option, we don't add the expression words to the vector when they fall within the window scope PLUS any word the fall in the window scope is added only once.
 *  FOR THIS REASON we store them in unordered_set<string> contextSet.
 *
 *
 *
 * 2- windowIncludeWordsINexpr and windowMultipleOccurrencesPosition
 * This is the original case if option is (1,1).
 * In this option, we add to the vector any repetitions either coming from the expression words e.g. <cat>
 *  and repetitions coming from any words (which are not part of the expressions) that fall repeatedly in the window scope <am>.
 * FOR THIS REASON we store them in vector<string> contextVector.
 *
 *
 *
 *
 * 3- windowIncludeWordsINexpr and NOT windowMultipleOccurrencesPosition
 *This case if the option is (1,0).
 *In this option, we add to unordered_set<string> contextSet the words from the expression words e.g. <cat>  that fall in the window scope and we only add once
 *any words that fall repeatedly in the window scope <am>.
 *
 *
 *
 *
 *
 * 4- currentWordIsNOTInExpr and windowMultipleOccurrencesPosition
 *This case if the option is (0,1).
 * In this option, we don't add the expression words to the vector when they fall within the window scope BUT we add
 * the repetitions coming from any words (which are not part of the expressions) that fall repeatedly in the window scope <am>.
 *
 *
 *

 *
 *
 *HERE we window scope but only related to expressions:
 * Option two with Windowsize and Including the expressions in any cases:
 *
 *1- To include the expressions words as default but don't allow any repetitions

 *2- OR to include them and allow repetitions only of the expressions words.
 *
 *
 *
 *
 * Option three with NOWindowsize:
 *
 * if halfWindowSize is set to 0 by the user, this is interpreted as a special case where we consider the whole sentence
 * as the context window for the expression. Depending on the two previous options, words from the expression are included or not and
 * the vector contains repetitions or not.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */














void Context::addToContext(int sentenceId, vector<int> &positions){


	if (corpus != NULL) {
		if (!positions.empty()) { // ignore if expr is not in the sentence
			Sentence &s = corpus->getSentence(sentenceId);// getting the sentnece itself from the ID.
			freq++;// how many sentences we have for this expression.

			vector<string> words = s.getwords();//sentence from europal
			unordered_set<string> contextSet;
			vector<string> contextVector;

			for (int i=0; i<positions.size(); i++) { // remark: in multi words expr the other words of the expr might be included (possibly even several times)
				//			cerr << "i="<< i << endl;
				if (halfWindowSize>0) {

					for (int windowPos=positions[i]-halfWindowSize; (windowPos<s.getwords().size()) && (windowPos<=positions[i]+halfWindowSize); windowPos++) {
						if ((windowPos>=0) && (windowPos!=positions[i])) {// windowPos<o segmentation fault.( if the expression was at the start of the sentence and we don't want to take the expression into account)

							if (windowIncludeWordsINexpr || (find(positions.begin(), positions.end(), windowPos) == positions.end())) {
								// the condition above is equivalent to the following two lines:
								//							int currentWordIsNOTInExpr = find(positions.begin(), positions.end(), windowPos) == positions.end();
								//							if (windowIncludeWordsINexpr || currentWordIsNOTInExpr) {
								if (corpus->isContextVocabWord(words[windowPos])) {// to avoid stopwords
									//							cerr << "L word="<< words[l] << endl;
									if (windowMultipleOccurrencesPosition) {
										contextVector.push_back(words[windowPos]);
									} else {
										contextSet.insert(words[windowPos]);
									}
								}
							}
						}
					}
					// I'm not sure that we really need this case (no window size, take all the words in the sentence)
				} else { // if window size = 0, take all the words, except the ones in expr
					if (windowIncludeWordsINexpr || (find(positions.begin(), positions.end(), i) == positions.end())) { // if i is not one of the indexes of the expression
						if (corpus->isContextVocabWord(words[i])) {
							if (windowMultipleOccurrencesPosition) {
								contextVector.push_back(words[i]);
							} else {
								contextSet.insert(words[i]);
							}
						}
					}
				}
			}
//*************** end of for loop***********************


			if (!windowMultipleOccurrencesPosition) {
				contextVector = vector<string>(contextSet.begin(), contextSet.end()); // copy the words in the set in the vector
			}

			addWordsToContext(contextVector);// created the full expression context = cooc for full exp



		}
	} else {
		cerr << "Bug: corpus must not be NULL here (addToContext(int, vector<int>))\n";
		exit(7);
	}
}






// here we are filling cooc with the words that we found from the window. so that cooc is our real context vector.

void Context::addWordsToContext(vector<string> &words) {// context words from window without the expressions.

	double wordValue;
	for (int i=0; i < words.size(); i++) {
		//	cerr << "adding word '"<<words[i]<<"' to cntext"<<endl;
		string &word=words[i];
		if (normalizeByExprOccurrence) {
			wordValue = 1.0 / words.size();// words.size() = context vector// relative frequency.
		} else {
			wordValue = 1.0;// absolute frequency
		}
		//		cerr << "adding word "<<words[i]<<"' to cntext ; wordValue= "<<wordValue <<endl;
		std::unordered_map<string,double>::iterator found = cooc.find(word);// here were we just do cooc
		if ( found == cooc.end() ) {
			//cerr << "new"<<endl;
			cooc[word] = wordValue;
		} else {
			//cerr << "prev value = "<< found->second<<endl;
			found->second += wordValue;
		}
	}
}

//The first one only counts the number of different words in common (i.e. which appear in both) between the two contexts
//	and divides by the total number of different words.
//	With the example of "computer" and "cat" we have "sleep" and "mouse" in common = 2,
//	and the total of different words is 8 so the result is 2/8 = 0.25


// to count inside the map
double Context::simJaccard(Context &context2){

	int count = 0;
	int tot_count =0;

	for ( auto it = cooc.begin(); it != cooc.end(); ++it){// this.cooc vs context2.cooc

		unordered_map<string,double>::iterator found = context2.cooc.find(it->first);


		if(found == context2.cooc.end()){// true if its not found

			tot_count ++; /// the for loop loops over the full expression cooc

		}else {

			count++;
			tot_count++;
		}
	}

	//2nd loop for counting the words which are in context2 but not in the current context (they are counted in the union)
	for( auto it = context2.cooc.begin(); it != context2.cooc.end(); ++it){

		unordered_map<string,double>::iterator found = this->cooc.find(it->first);


		if(found == this->cooc.end()){// if the pointer equals to the end means we dont find it.


			tot_count++;
		}
	}

	double result = count/tot_count++; // the idea is the more words in common between the two context vectors, the more similar these vector contexts are.

	return result;
}


//for the second one we take each distinct word one by one, we sum the minimum frequency values together
//and the maximum frequency values together, and then we divide the min sum by the max sum.
//if a word doesn't appear it means its frequency is zero. example with "computer" vs "cat":

double Context::simMinMax(Context &context2){


	double min = 0;
	double max = 0;

	for ( auto it = this->cooc.begin(); it != this->cooc.end(); ++it){

		unordered_map<string,double>::iterator found = context2.cooc.find(it->first);
		// here we are not sure if it founds or not.

		if (found == context2.cooc.end()){ // word not found in context2

			max = it->second + max;

		} else { // word found

			if(it->second < found->second){

				min = it->second + min;

				max = found->second + max;
			} else {

				min = found->second + min;
				max = it->second + max;
			}
		}
	}

	// testing all the words that exist in context2 but not in this object.
	for( unordered_map<string,double>::iterator  it = context2.cooc.begin(); it != context2.cooc.end(); ++it){

		unordered_map<string,double>::iterator found = this->cooc.find(it->first);

		if(found == cooc.end()){ // the object found iterates over this current object.

			max = it->second + max;
		}
	}

	double result = min/max;

	return result;
}





double Context::simCosine(Context &context2, bool withIDF){

	double norm1=0;
	double norm2=0;
	double sumprod=0;

	if (corpus != NULL) {
		for ( auto it = this->cooc.begin(); it != this->cooc.end(); ++it){

			double v1 = it->second;
			if (withIDF) {
				v1 *= corpus->getIDF(it->first);
			}
			norm1 += pow(v1,2);

			unordered_map<string,double>::iterator found = context2.cooc.find(it->first);
			// here we are not sure if it founds or not.

			if (found != context2.cooc.end()){ // word not found in context2
				double v2 = found->second;
				if (withIDF) {
					v2 *= corpus->getIDF(found->first);
				}
				sumprod += v1 * v2;
				norm2 += pow(v2, 2);
			}
		}

		// testing all the words that exist in context2 but not in this object.
		for( unordered_map<string,double>::iterator  it = context2.cooc.begin(); it != context2.cooc.end(); ++it){

			unordered_map<string,double>::iterator found = this->cooc.find(it->first);

			if(found == cooc.end()){ // the object found iterates over this current object.
				double v2 = it->second;
				if (withIDF) {
					v2 *= corpus->getIDF(it->first);
				}
				norm2 += pow(v2, 2);
			}
		}


		double result = sumprod / (sqrt(norm1)*sqrt(norm2));

		return result;
	} else {
		cerr << "Bug: corpus must not be NULL here (simCosine)\n";
		exit(7);
	}

}



// print the list of words
//print the
void Context::prntContext(bool prntSentences){
	cout<< "list of" << cooc.size() << "words with thier frqs in cooc: "<< endl;


	for(unordered_map<string,double>::iterator it = cooc.begin(); it != cooc.end(); ++it){

		cout << it->first << ":" <<it->second << '\n';

	}
	cout << "found in "<< freq <<" sentences."<<endl;
	/* OBSOLETE
	if(prntSentences == true){

		for (int i : inSentences) {
			//		for (unordered_set<int>::iterator it = inSentences.begin() && it != inSentences.end(); ++it){ not sure how to use this way!

			Sentence& s = corpus.getSentence(i);

			s.prnt_wrd();


		}
	}
	 */

}

// we give a content of the pointer but its received as an address so we don't have to copy the whole content again.

double Context::computeSimScore(Context &context2) {

	if (corpus != NULL) {
		string &simMeasureId = corpus->getSimMeasureId();
		if (simMeasureId == "jaccard") {
			return simJaccard(context2);
		} else {
			if (simMeasureId == "minmax") {
				return simMinMax(context2);
			} else {
				if (simMeasureId == "cosine") {
					return simCosine(context2, false);
				} else {
					if (simMeasureId == "cosineIDF") {
						return simCosine(context2, true);
					} else {
						cerr << "Error: invalid sim measure id: '"<<simMeasureId<<"'"<<endl;
					}
				}
			}
		}
	} else {
		cerr << "Bug: corpus must not be NULL here (computeSimScore)\n";
		exit(7);
	}
}



/*
 * Computes a group of features by comparing the current expression against a set of other expressions
 * The function is used with the other expressions being either pseudo-expressions (a part of the whole expression) or alternative expressions (from alternative sequences)
 *
 */

// probe is what we test against the reference expression.
vector<double> Context::featuresComparison(vector<Context *> probe, string featTypes, bool featOnlyMean) {// please DONT forget about the main loops that brought you here with the current FULL expression

  vector<double> res;
  if (featTypes[0] != '0') {
	double freqRef = (double) getFrequency();// the absolute freq of the current full expression from the reference corpus
	// frequency features: for every context vector in probe, ratio of the frequency of the current vector with the probe vector
	// rationale: high value if the current vector is more frequent and conversely (with 1 = equal frequency)
	vector<double> freq(probe.size());// to store the result of the division.
	for (int i=0; i<probe.size(); i++) {
		//	  cout << "debug probe expr "<< i <<" = " << probe[i]->getExprAsString() << "; freqref="<<freqRef<<"; freq probe="<<probe[i]->getFrequency()<<"\n";
		freq[i] = freqRef / (double) probe[i]->getFrequency(); // the result of ratios
	}
	//	cout << "DEBUG: freq comparison vector = ";
	//	printDoubleVector(freq);
	//	cout << "\n";
	vector<double> featuresFreq = vectorStats(freq, featOnlyMean);
	res.insert(res.end(),featuresFreq.begin(), featuresFreq.end());
  }


  if (featTypes[1] != '0') {
	// similarity features: for every context vector in probe, similarity score of the current vector with the probe vector
	vector<double> simScores(probe.size());
	for (int i=0; i<probe.size(); i++) {
		simScores[i] = computeSimScore(*probe[i]);// the vector context for the individual persudo expression??
	}
	//	cout << "DEBUG: sim comparison vector = ";
	//	printDoubleVector(simScores);
	//	cout << "\n";
	vector<double> featuresSim = vectorStats(simScores, featOnlyMean);
	res.insert(res.end(),featuresSim.begin(), featuresSim.end());
  }

  return res;

}

// the result of the comparisons between one of the two groups and the actual candidate is the features
/*
 * Compute internal features, i.e. two groups:
 * (1) pseudo-expressions made of the individual words in the expression
 * (2) pseudo-expressions made of the whole expression minus one word
 *
 * In both cases the pseudo-expression is compared to the whole expression (rationale: measure how much each part is related to the whole expression)
 *
 */

// because the context class doesnt have an access to the context collection class, we need to provide the collection as a parameter.
void Context::computeInternalFeatures(ContextCollection *contextColl, string featGroups, string featTypes, string featStd, bool featOnlyMean) {

	//cerr << "debug: start computeInternalFeatures for '"<< joinStrings(expr,";") <<"'"<<endl;
	internalFeatures.clear();// where we put our features
	if (expr.size()>1) {// the actual candidate


		// 1) build indiv words and expr - 1 word contexts
		vector<Context *> singleWordContexts;
		vector<Context *> allButOneWordContexts;
		for (int i=0; i<expr.size(); i++) {

			vector<string> expr1word;
			expr1word.push_back(expr[i]);// this vector contains the words of pusedo expressions and is used to build the context object for them so we store the context object address in the vector <Context *>.
			Context *c = NULL;
			if (contextColl != NULL) {
				c = contextColl->findInColl(expr[i]); // returns NULL if not found
			}
			if (c == NULL) {// we are building and calculating the context vector for the pusedo expression word.
				c = new Context(expr1word, halfWindowSize, 0, normalizeByExprOccurrence, windowIncludeWordsINexpr, windowMultipleOccurrencesPosition, corpus);

				if (contextColl != NULL) {
					contextColl->addContext(c);
				}
			}
			singleWordContexts.push_back(c);
			//	    printStringVector(expr1word);
			//	    cout << " ; frequency = " << singleWordContext.getFrequency() << " ; conditional prob = " <<  (double) getFrequency() / (double) singleWordContext.getFrequency() <<endl ;

			vector<string> exprAllBut1; //this vector contains the words of pusedo expressions to be used to build the context object for them
			for (int j=0; j<expr.size(); j++) {
				if (i!=j) { // we cad do minus one pusedo expressions.
					exprAllBut1.push_back(expr[j]);
				}
			}


			//// we are building and calculating the context vector for the pusedo expression word......

			if (contextColl != NULL) {
				c = contextColl->findInColl(exprAllBut1); // returns NULL if not found
			}
			if (c == NULL) {
				c = new Context(exprAllBut1, halfWindowSize, 0, normalizeByExprOccurrence, windowIncludeWordsINexpr, windowMultipleOccurrencesPosition,corpus);
				if (contextColl != NULL) {
					contextColl->addContext(c);
				}
			}
			allButOneWordContexts.push_back(c);
			//	    cout << " Expr minus word = ";
			//	    printStringVector(exprAllBut1);
			//	    cout << " ; frequency = " << allButOneContext.getFrequency() << " ; conditional prob = " <<  (double) getFrequency() / (double) singleWordContext.getFrequency() <<endl ;
		}// after this loop we have added both groups to the object context collection in the exprs map.
		//	also we have filled the two "group" vectors singleWordContexts, and allButOneWordContexts which contain the Context addresses for all the
		// pseudo-expressions

		// here we are doing the comparisons between the context vectors for the first two groups.
		if (corpus != NULL) {

			// 2) compute features for indiv words contexts
			//    cout << "DEBUG ####### current expression = ";
			//    printStringVector(expr);
			//    cout << "\n";
			//    cout << "DEBUG: ### comparison single word\n";
		  vector<double> featsSingle = featuresComparison(singleWordContexts, featTypes, featOnlyMean);
			// 3) compute features for expr-1word contexts
			//    cout << "DEBUG: ### comparison expr minus one\n";
			vector<double> featsMinus1 = featuresComparison(allButOneWordContexts, featTypes, featOnlyMean);


			// first feature = number of words
			if (featStd[1] != '0') {
			  internalFeatures.push_back(expr.size());
			}

			// second feature = relative frequency of the expression in the ref corpus
			// update: finally decided that absolute frequency is more convenient here
			if (featStd[2] != '0') {
			  internalFeatures.push_back((double) getFrequency());
			}
			//    internalFeatures.push_back((double) getFrequency() / corpus.nbSentences());

		  if (featGroups[0] != '0') {
		    internalFeatures.insert(internalFeatures.end(), featsSingle.begin(), featsSingle.end());
		  }
		  if (featGroups[1] != '0') {
		    internalFeatures.insert(internalFeatures.end(), featsMinus1.begin(), featsMinus1.end());
		  }
		} else {
			//      cerr << "Info: not computing features here\n";
		}
	} else {
		if (corpus != NULL) {
		  vector<double> v = zeroLengthInternalFeatures(1, (double) getFrequency() / corpus->nbSentences(), featGroups, featTypes, featStd, featOnlyMean);
			internalFeatures.insert(internalFeatures.end(), v.begin(), v.end());
		} else {
			//      cerr << "Info: not computing features here\n";
		}
	}

}




/*
 * Compute external features, i.e. group of features based on comparing the current expression to a set of other expressions (typically from alternative sequences)
 *
 */

vector<double> Context::externalFeatures(vector<Context *> otherPossibleContexts, string featTypes, string featStd, bool featOnlyMean) {
	vector<double> res;

	if (otherPossibleContexts.size() == 0) {
	  return zeroLengthExternalFeatures(featTypes, featStd, featOnlyMean);
	} else {
		//    cout << "DEBUG: ### comparison alternative expressions\n";
	  vector<double> featsOthers = featuresComparison(otherPossibleContexts, featTypes, featOnlyMean);
	  if (featStd[3] != '0') {
	    res.push_back(otherPossibleContexts.size()); // number of "others"
	  }
		res.insert(res.end(), featsOthers.begin(), featsOthers.end());
		return res;

	}


}


int Context::hasInternalFeatures() {
	return (internalFeatures.size()>0);
}


vector<double> &Context::getInternalFeatures() {
  // update: removed the following sanity check because (1) this problem does not happen anymore and (2) now that features can be selected, it is possible that the vector
  //         internalFeatures has size 0 "normally", i.e. simply because no feature has been selected at all.
  //	if (internalFeatures.size()==0) {
  //		// remark: normally this can only happen for sub-expressions in the case of a ContextCollection, in other words it should not happen since we don't need
  //		// the features for sub-expressions (only for actual candidate expressions)
  //		cerr << "Error: getInternalFeatures called but internalFeatures have not been computed, this is not allowed in the current design (translation: this is a bug)."<<endl;
  //		exit(5);
  //		//    computeInternalFeatures();
  //    }
	return internalFeatures;
}





// non-class functions


vector<double> zeroLengthInternalFeatures(string featGroups, string featTypes, string featStd, bool featOnlyMean) {
  return zeroLengthInternalFeatures(0, 0, featGroups, featTypes, featStd, featOnlyMean);
}


vector<double> zeroLengthInternalFeatures(int length, double exprFreq, string featGroups, string featTypes, string featStd, bool featOnlyMean) {
	vector<double> res;
	if (featStd[1] != '0') {
	  res.push_back((double) length);
	}
	if (featStd[2] != '0') {
	  res.push_back(exprFreq);
	}
	int nbTypes=0;
	if (featTypes[0] != '0') {
	  nbTypes++;
	}
	if (featTypes[1] != '0') {
	  nbTypes++;
	}
	int nbGroups=0;
	if (featGroups[0] != '0') {
	  nbGroups += 1;
	} 
	if (featGroups[1] != '0') {
	  nbGroups += 1;
	}
	int nbFeats = nbTypes * nbGroups;
	if (!featOnlyMean) {
	  nbFeats *= 3;
	} // else same ( * 1)
	for (int i=0; i<nbFeats; i++) {
		res.push_back(std::numeric_limits<double>::quiet_NaN());
	}
	return res;

}


vector<double> zeroLengthExternalFeatures(string featTypes, string featStd, bool featOnlyMean) {
	vector<double> res;
	if (featStd[3] != '0') {
	  res.push_back(0.0);
	}

	int nbFeats=0;
	if (featTypes[0] != '0') {
	  nbFeats++;
	}
	if (featTypes[1] != '0') {
	  nbFeats++;
	}
	if (!featOnlyMean) {
	  nbFeats *= 3;
	} // else same ( * 1)

	for (int i=0; i<nbFeats; i++) {
		res.push_back(std::numeric_limits<double>::quiet_NaN());
	}
	return res;
}


vector <string> internalFeaturesNames(string featGroups, string featTypes, string featStd, bool featOnlyMean) {

  const vector<string> groupId = {"indivWord", "exprMinusOneWord"};
  const vector<string> typeId = {"Freq", "Sim"};

  vector<string> names;

  if (featStd[1] != '0') {
    names.push_back("nbWords");
  }
  if (featStd[2] != '0') {
    names.push_back("freqExprRefCorpus");
  }

  for (int fGroup=0; fGroup<2; fGroup++) {
    //    cout << "DEBUG fGroup="<<fGroup<<"; featGroups[fGroup]="<<featGroups[fGroup]<<"; " <<endl;
    if (featGroups[fGroup] != '0') {
      for (int fType=0; fType<2; fType++) {
	//	cout << "DEBUG fType="<<fType<<"; featTypes[fType]="<<featTypes[fType]<<"; " <<endl;
	if (featTypes[fType] != '0') {
	  if (featOnlyMean) {
	    names.push_back(groupId[fGroup]+typeId[fType]+"Mean");
	  } else {
	    names.push_back(groupId[fGroup]+typeId[fType]+"Min");
	    names.push_back(groupId[fGroup]+typeId[fType]+"Mean");
	    names.push_back(groupId[fGroup]+typeId[fType]+"Max");
	  }

	}
      }

    }
  }
  return names;
}


vector<string> externalFeaturesNames(string featTypes, string featStd, bool featOnlyMean) {

  const vector<string> typeId = {"alternativeExprsFreq", "alternativeExprsSim"};

  vector<string> names;
  if (featStd[3] != '0') {
    names.push_back("nbAlternativeExprs");
  }


  for (int fType=0; fType<2; fType++) {
    if (featTypes[fType] != '0') {
      if (featOnlyMean) {
	names.push_back(typeId[fType]+"Mean");
      } else {
	names.push_back(typeId[fType]+"Min");
	names.push_back(typeId[fType]+"Mean");
	names.push_back(typeId[fType]+"Max");
      }

    }
  }
  return names;
}
