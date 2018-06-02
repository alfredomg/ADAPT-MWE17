/*
 * outcomes.cpp
 *
 *  Created on: 26 Jan 2017
 *      Author: ash
 */

#include "outcomes.h"
#include <locale>
#include <unordered_map>
#include "misc.h"
#include <vector>
#include<string>

outcomes::outcomes() {
  exprNoForBIOFormat = 0;
  expression.clear();
}

outcomes::outcomes(int id, double confi) {
  exprNoForBIOFormat = 0;
   sentenceID = id;
   //   cout << "sentID" <<sentenceID << endl;



	confidence_score = confi;
//	cout << "conf=" <<confidence_score << endl;



	expression.clear();



}





void outcomes::addWord(string w, string label) {
	locale loc;

	//	cout << "addWord: word="<<w <<"; label= "<<label << endl;
	if (label != "_"){

	  if (isdigit(label[0], loc) || (label == "B") || (label == "I")) { // valid labels: either int number or B or I (from BIO format)
	    int lab;
	    if (isdigit(label[0], loc)) {
	      lab = stoi(label);
	    } else { // label == B or label == I
	      //	      cout << "label="<<label<<" for word "<<w<<"\n";
	      // increase expression number if meeting new expression in BIO format, except if it's the first in the sentence
	      // this way the first expression has number 0 whether it starts with label B  (as it's supposed to) or I (frequent in CRF predictions)
	      if ((label == "B") && expression.size()>0) { 
		exprNoForBIOFormat++;
	      }
	      lab = exprNoForBIOFormat;
	    }
	    //	    cout << "new tagged word: word="<<w <<"; label= "<<label<<"; lab=" << lab << endl;

	    unordered_map<int, vector<string>>::iterator found = expression.find(lab);
	    
	    if(found == expression.end()){

	      vector<string> exprssns (1,w);// push back already. (size, the input).
	      expression[lab] = exprssns;

	    }else{

	      pushback(found->second, w);
	      //				found->second.push_back(w);
	    }
	  } else {

	    cout << "Invalid label: tt's not _, not a number and not B or I !" << endl;
	    exit(1);
	  }
	  //		print();

	}
}


void outcomes::pushback(vector<string> &v, string w) {

		v.push_back(w);

}


void outcomes::print() {
  cout << " ["<< expression.size()<<" exprs] ";
	for (auto it = expression.begin(); it != expression.end(); ++it){
	  cout <<"   (no "<< it->first << ") : " ;
	printStringVector(it->second);
	//	cout << endl;
	}

}

unordered_map<int, vector<string>> &outcomes::getExprMap() {
  return expression;
}

unordered_map<int, Context *> &outcomes::getContextMap() {
  return contexts;
}

double outcomes::getConfidence() {
  return confidence_score;
}

int outcomes::getSentId() {
  return sentenceID;
}

void outcomes::setContext(int label, Context *c) {// when context collection takes the expresssion create the context object for this expression and then it tells the original outcome object it gives back the pointer with this function
  contexts[label] = c;
  //  cerr <<"debug: "<< this<<" setting context in outcomes, new size = " << contexts.size() << "\n";
}


bool outcomes::sameExprs(outcomes &other) {
  if (expression.size() != other.expression.size()) {
    return false;
  }
  for (auto it: expression) {
    auto found = other.expression.find(it.first);
    if (found == other.expression.end()) {
      return false;
    } else {
      vector<string> thisExpr = it.second;
      vector<string> otherExpr = found->second;
      if (thisExpr.size() != otherExpr.size()) {
	return false;
      }
      for (int i=0; i< thisExpr.size(); i++) {
	if (thisExpr[i] != otherExpr[i]) {
	  return false;
	}
      }
    }
  }
  return true;
  
}
