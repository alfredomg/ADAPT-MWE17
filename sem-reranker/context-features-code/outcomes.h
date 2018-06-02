/*
 * outcomes.h
 *
 *  Created on: 26 Jan 2017
 *      Author: ash
 */

#ifndef OUTCOMES_H_
#define OUTCOMES_H_


#include<string>
#include<vector>
#include <locale>
#include<unordered_map>
#include<fstream>
#include<iostream>
#include "Context.h"
using namespace std;



class outcomes {
public:
	outcomes();
	outcomes(int id, double confi);
	void addWord(string w, string label);
	void pushback(vector<string> &v, string w);
	void print();
	unordered_map<int, vector<string>> &getExprMap();
	unordered_map<int, Context *> &getContextMap();
	double getConfidence();
	int getSentId();
	void setContext(int label, Context *c);
	bool sameExprs(outcomes &other);
	
private:
	int exprNoForBIOFormat;
    int sentenceID;
	double confidence_score;
	unordered_map<int, vector<string>> expression; // stores the predicted label of the expression: the expressions for that label in one possibilities of 10 for a sentence.
	unordered_map<int, Context*> contexts;

};

#endif /* OUTCOMES_H_ */
