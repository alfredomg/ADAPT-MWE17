/*
 * MWE.cpp
 *
 *  Created on: 14 Jan 2017
 *      Author: ash
 */



#include "Sentence.h"
#include <iostream>
#include<istream>
#include <string>
#include <locale>
#include <vector>

Sentence::Sentence(void) {

	tokens.clear();



			}



void Sentence::addwords(string tkn){

	tokens.push_back(tkn);

}

void Sentence::prnt_wrd(void){

//	cout << tokens.size();
	for(int i = 0; i <= tokens.size()-1; i++){

     cout << this->tokens[i] << "/";


	}
	cout << endl;

}



void Sentence::clear(){

	tokens.clear();


	}



vector <string>& Sentence::getwords(void){


	return tokens;

}



// class Sentence!!
/*
 * searches an multiword expressions expr in the sentence object, and returns a pointer to a vector of positions where each index n gives the position of the nth word of the expression.
 * If maxDist > 0, then additionally the distance between the first and last word in the expression must not be higher than maxDist.
 *
 * returns empty vector if the expr is not found, or not found with total length <= maxDist.
 *
 */

// this function will be only called for one sentence since we are in the class sentence
vector<int> Sentence::findExpression(vector<string> &expr, int maxDist) {
  int indexExpr = 0;
  int startIndexSent = 0;
  vector<int> positions(expr.size());// calling the constructor for the vector insialised with the exact expr size.

  do {
    int indexSent = startIndexSent; // if not the first time we do this loop, startIndexSent has been increased: we try to find the expr starting further on the right

    while ((indexSent<tokens.size()) && (indexExpr < expr.size())) { // && ((positions.size() <= 1) || ((maxDist==0) || ((positions.back() - positions.front())  <= maxDist)))) {

      // find next matching word
      while ((indexSent<tokens.size()) && (tokens[indexSent] != expr[indexExpr])) {
        indexSent++;
      }
      if (indexSent<tokens.size()) { // found matching word, storing position and moving to next word in expression
        positions[indexExpr] = indexSent;
        indexExpr++;
        indexSent++;
      }

    }

    // do it again if the following conditions are all true:
    // - maxDist>0, i.e. there is a maximum length to satisfy
    // - there is still enought words before the end of the tokens in the sentence
    // - we found the expression: indexExpr == expr.size(), butthe actual length found doesn't satisfy the max length condition
  } while ( (maxDist > 0) && (++startIndexSent <  tokens.size()-expr.size()) && (indexExpr == expr.size()) &&  (positions.back() - positions.front()  > maxDist));
  // in any other cases we leave the loop:
  // - no maxDist constraint to satisfy
  // - reached the end of the tokens without finding the expr within maxDist cndition

  if ( (indexExpr < expr.size()) || ( (maxDist > 0) && (positions.back() - positions.front() > maxDist)) ) { // failed to find the expr
    positions.clear(); // empty vector means there is no expression.
  }
  return positions;



}
