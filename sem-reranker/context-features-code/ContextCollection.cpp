
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ContextCollection.h"

using namespace std;

// remark: a reference must be initialized immediately when it's declared, but in the case of a variable member of a class it does not exist before the call to the constructor.
//         This is why we must we the "initialization list" (special assignment instructions done just before the body of the constructor)
// see http://en.cppreference.com/w/cpp/language/initializer_list, 
ContextCollection::ContextCollection(annotatedcorpus &data , unordered_map<string, string> options, Corpus *corpusIn, bool training) : corpus(corpusIn), annData(data) {// we are not allowed to inislised these two list inside ebcause its a reference variables.

  trainingMode = training;
  exprs.clear();
  minConfidence = stod(options["minConfidence"]);
  this->halfWindowSize = stoi(options["halfWindowSize"]);
  this->normalizeByExprOccurrence = stoi(options["normalizeContext"]);
  this->windowIncludeWordsINexpr = stoi(options["windowIncludeWordsINexpr"]);
  this->windowMultipleOccurrencesPosition= stoi(options["windowMultipleOccurrencesPosition"]);
  this->featGroups = options["featGroups"];
  this->featTypes = options["featTypes"];
  this->featStd = options["featStd"];
  this->featOnlyMean = stoi(options["featsOnlyMean"]);


	cout <<"Computing context for expressions..."<<endl;
	// iterating over all "outcomes" to collect expressions
	//  vector<annotatedsentence> sentences = data.getAnnotatedSentences(); // err: copy object!!
	int total = data.nbSentences();
	for (int sentNo=0; sentNo<total; sentNo++) {// we loop over all the anoonated crf sentnecs

		//    cerr << "info: processing annotated sentence: " << sentNo<< " / " << sentences.size() << endl;
		cout << "\r" << sentNo<< " / " << total;
		annotatedsentence *s = data.getSentence(sentNo);
		// update: we don't need to calculate context vectors (and features) for the gold sequence currently, hence comments below
		// if (trainingMode) {
		//	//      cerr <<"debug: gold processing\n";
		//	outcomes *g=s->getGold();// 1-******************to create outcomes object
		//	addAllExprsInMap(g);/// !!!!repetition???improve it....
		//}
		for (int i=0; i<s->getNbOutcomes(); i++) {// here this one for sequences of crf not sentences
			//      cerr <<"debug: outcomes "<<i<<" processing\n";
			outcomes *o = s->getOutcomes(i);
			if (o->getConfidence()>=minConfidence) {
				addAllExprsInMap(o);// we call this function for each sequence
			}
		}
	}
	cout << endl;
}

// to create 1- context object and add them to the map in context collection exprs
void ContextCollection::addAllExprsInMap(outcomes *o) {
	for (auto it : o->getExprMap()) {// to loop over the expressions in the map for one sequence
		vector<string> &words = it.second;
		if (words.size()>0) {
			string exprStr  = joinStrings(words, " ");// instead of haviing a vector we want to make them one expression
			unordered_map<string,Context*>::iterator foundExpr = exprs.find(exprStr);
			Context *c;
			if (foundExpr == exprs.end()) { // expr not yet in the map: initialize context object// here the big collection in case we find it in context collection exprs map or not.
				//	cout << "info: initializing context for expr '" << exprStr << "' words[0]='"<<words[0]<<"'" <<endl;
				c = new Context(words, halfWindowSize, 0, normalizeByExprOccurrence, windowIncludeWordsINexpr, windowMultipleOccurrencesPosition, corpus);
				exprs[exprStr] = c; // [key] value assignment to the map
			} else {
				c = foundExpr->second;
			}
			if (!c->hasInternalFeatures()) { // remark: either c is a new expression we just added, or it was already in the collection
				// but its features were not computed yet. The latter happens when a pseudo-expression was created and added
				// to the collection (without calculating features), but later we find the same as an actual candidate
				// expression so we need to calculate the features.
			  c->computeInternalFeatures(this, featGroups, featTypes, featStd, featOnlyMean);
			}
			//      cerr <<"debug set context\n";
			o->setContext(it.first, exprs[exprStr]);// we are filling the outcomes object with the context address  here
		} else {
			o->setContext(it.first, NULL);
		}
	}

}



Context *ContextCollection::findInColl(vector<string> expr) {
	string exprAsString = joinStrings(expr, " ");
	return findInColl(exprAsString);
}

Context *ContextCollection::findInColl(string exprAsString) {
	auto found = exprs.find(exprAsString);
	if (found == exprs.end()) {
		//    cerr << "debug: expr '" << exprAsString<<"' not found\n";
		return NULL;
	} else {
		//    cerr << "debug: expr '" << exprAsString<<"' found\n";
		return found->second;
	}
}

// to add a context to exprs map whatever the type of the expression is (individual, one mins or the third group)
void ContextCollection::addContext(Context *c) {
	string exprAsString = c->getExprAsString();
	auto found = exprs.find(exprAsString);
	if (found == exprs.end()) {
		//    cerr << "Adding context to collection for '"<<exprAsString<<"'\n";
		exprs[exprAsString] = c; // here we are adding the value as well as the key.
	} else {
		//    cerr << "Warning: context already in the collection, not adding it\n";
	}
}



int ContextCollection::size() {
	return exprs.size();
}

void ContextCollection::printExpressions(string filename) {

	ofstream fileStream(filename);
	for (auto it : exprs) {
		// still have to deal with POS!!!
		fileStream << it.first << endl;
	}
	fileStream.close();
}
