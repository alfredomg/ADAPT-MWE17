/*
 * annotatedsentence.cpp
 *
 *  Created on: 26 Jan 2017
 *      Author: ash
 */

#include "annotatedsentence.h"
#include "misc.h"

annotatedsentence::annotatedsentence() {
  gold = outcomes();
  clear();

}

annotatedsentence::~annotatedsentence() {
	// TODO Auto-generated destructor stub
}

bool annotatedsentence::isEmpty() {
	return pssble_sntncs.empty();
}

void annotatedsentence::addOutcome(int id, double confi) {
	//cout << "confi="<<confi<<endl;
	outcomes currentOutcome = outcomes(id, confi);
	pssble_sntncs.push_back(currentOutcome);
}

void annotatedsentence::addWord(string word, string goldlabel, string predict) {
	if (pssble_sntncs.size()==1) { // first outcome: store the gold answer as well
	  //	  	 		cout << "GOLD"<<endl;
		gold.addWord(word,goldlabel);

	}
	//			cout << "PREDICT "<<pssble_sntncs.size()-1 <<endl;
	pssble_sntncs[pssble_sntncs.size()-1].addWord(word,predict);
}

void annotatedsentence::clear() {
	pssble_sntncs.clear();
	gold= outcomes();
}


outcomes *annotatedsentence::getGold() {
  //  cerr <<"getGold = "<<&gold<<endl;
  return &gold;
}

vector<outcomes> &annotatedsentence::getOutcomes() {
  return pssble_sntncs;
}

outcomes *annotatedsentence::getOutcomes(int i) {
  return &pssble_sntncs[i];
}

int annotatedsentence::getNbOutcomes() {
  return pssble_sntncs.size();
}


vector<vector<double>> annotatedsentence::computeFeatures(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean, bool trainingMode, int sentNo) {

  unordered_map<string, Context *> allExprs;
  if (featGroups[2] != '0') {
    //cerr << "**start ann sent features for sentNo="<<sentNo<<"\n";
    for (int i=0; i<pssble_sntncs.size(); i++) { // pass 1: create a map with all the expressions
      unordered_map<int, Context *> m = pssble_sntncs[i].getContextMap();
      for (auto it : m) {
	Context *c = it.second;
	if (c != NULL) { // ignore otherwise
	  string exprStr = c->getExprAsString();
	  allExprs[exprStr] = c;
	}
      }
    }
  }

    int foundGold = -1;
    vector<vector<double>> allFeatures;
    // pass 2: compute external features, which means compare to the other other contexts (which means exclude the probed one)
    // cerr << " tset gold "<<&gold <<" nb exprs =" << gold.getContextMap().size()<< endl;
    for (int i=0; i<pssble_sntncs.size(); i++) {
      unordered_map<int, Context *> m = pssble_sntncs[i].getContextMap();
      //      cerr <<"outcome "<< i<<" = "<<&pssble_sntncs[i] << ": nb exprs = " << m.size()<< endl;
      vector<vector<double>> featsForAllExprsInOutcome;
      for (auto it : m) {
	//cerr<< "looping expr in alternative sequence "<< i <<"\n";
	Context *c = it.second;
	vector<double> internalFeats;
	vector<double> otherFeats;
	if (c != NULL) {
	  string exprStr = c->getExprAsString();
	  //cerr << "getting features for "<<exprStr<<endl;
	  internalFeats = c->getInternalFeatures();


	  if (featGroups[2] != '0') {
	    // build "others"
	    vector<Context *> others;
	    for (auto allExprsIt : allExprs) {
	      string otherExprStr = allExprsIt.first;
	      if (exprStr != otherExprStr) { // pass the current one
		Context *otherContext = allExprsIt.second;
		others.push_back(otherContext);
	      }
	      otherFeats = c->externalFeatures(others, featTypes, featStd, featsOnlyMean);
	    }
	  }
	} else { // expr with 0 words, should never happen!
	  cerr << "BUG: expression with zero words in annotated corpus????\n";
	  exit(15);
	  // to make it work, even though it doesn't make sense:
	  //	  internalFeats = zeroLengthInternalFeatures(featGroups, featTypes, featsOnlyMean);
	  //	  if (featGroups[2] != '0') {
	    //	    otherFeats = zeroLengthExternalFeatures(featTypes, featsOnlyMean);
	    //	  }
	}
	// concatenate features
	vector<double> features;
	features.insert(features.end(), internalFeats.begin(), internalFeats.end() );
	features.insert(features.end(), otherFeats.begin(), otherFeats.end() );  // normally otherFeats is empty if (featGroups[2] == '0')
	//	cout << "indiv expr feats vector ("<<features.size()<< ") = ";
	//printDoubleVector(features);
	//cout<<endl;
	featsForAllExprsInOutcome.push_back(features);
      } // end for all expressions in this sentence
      if (featsForAllExprsInOutcome.size()==0)  { // if the outcome contains no expr at all, we will give the stats with NaN values CAUTION: this will also happen if the outcome has confidence lower than min confidence!!
	// build dummy vector for legnth
	vector<double> features = zeroLengthInternalFeatures(featGroups, featTypes, featStd, featsOnlyMean);;
	if (featGroups[2] != '0') {
	  vector<double> otherFeats = zeroLengthExternalFeatures(featTypes, featStd, featsOnlyMean);
	  features.insert(features.end(), otherFeats.begin(), otherFeats.end() );
	}
	featsForAllExprsInOutcome.push_back(features);
      }
      vector<double> finalFeatsForOutcome;
      finalFeatsForOutcome.push_back((double) sentNo);
      finalFeatsForOutcome.push_back((double) pssble_sntncs[i].getSentId());
      if (featConfidence) {
	finalFeatsForOutcome.push_back(pssble_sntncs[i].getConfidence()); // feat 1 = confidence
      }
      if (featStd[0] != '0') {
	finalFeatsForOutcome.push_back(m.size()); // feat 2 = number of exprs
      }
      vector<double> stats = vectorStatsByColumn(featsForAllExprsInOutcome, featsOnlyMean);

      //cout << "outcomes stat feats vector ("<<stats.size()<< ") = ";
      //	printDoubleVector(stats);
      //	cout<<endl;

      finalFeatsForOutcome.insert(finalFeatsForOutcome.end(), stats.begin(), stats.end() );

      // include CLASS
      double classVal = 0.0;  // default value = negative
      if (trainingMode && pssble_sntncs[i].sameExprs(gold)) {  // if train mode AND POSITIVE
	//	  cout << "DEBUG: found gold sequence in alternative "<<i<<":";
	//	  pssble_sntncs[i].print();

	// this was a test when I didn't expect several sequences can be gold at the same time, but it turns out it's ok: we just assign one to every
	// predicted sequence which corresponds to the gold
	//
	//      	if (foundGold != -1) { // not supposed to happen, kind of as safety check
	//	  cerr <<"Warning! found gold labeling several times in the outcomes!!\n";
	//	  cerr << "  previous confidence = "<< pssble_sntncs[foundGold].getConfidence()<< " ; new = "<< pssble_sntncs[i].getConfidence()<<endl;
	//	} else {
	//	  foundGold = i;
	//	}
	
	classVal = 1.0;
      }
      finalFeatsForOutcome.push_back(classVal);
      allFeatures.push_back(finalFeatsForOutcome);
    } // end of pass 2
    return allFeatures;


}


vector<string> annotSentenceFeatNames(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean) {
  vector<string> names;
  names.push_back("sentNo");
  names.push_back("alternativeNo");
  if (featConfidence) {
    names.push_back("confidence");
  }
  if (featStd[0] != '0') {
    names.push_back("nbExpressions");
  }
  vector<string> indivExprNames = internalFeaturesNames(featGroups, featTypes, featStd, featsOnlyMean);
  if (featGroups[2] != '0') {
    vector<string> external = externalFeaturesNames(featTypes, featStd, featsOnlyMean);
    indivExprNames.insert(indivExprNames.end(), external.begin(), external.end());
  }
  vector<string> statsNames = vectorStatsByColumnNames(indivExprNames, featsOnlyMean);
  names.insert(names.end(),statsNames.begin(), statsNames.end());

  names.push_back("correctness"); // class name, why not
  return names;
}
