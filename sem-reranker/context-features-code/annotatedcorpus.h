/*
 * annotatedcorpus.h
 *
 *  Created on: 25 Jan 2017
 *      Author: ash
 */

#ifndef ANNOTATEDCORPUS_H_
#define ANNOTATEDCORPUS_H_

#include "annotatedsentence.h"
#include "outcomes.h"
#include<string>
#include<vector>
#include <locale>
#include<unordered_map>
//#include<fstream>
#include<iostream>

using namespace std;
// 1) similarly to what is done in Corpus, do another function which reads a corpus in a similar format (one word by line, several columns etc.;
// see alfredo's example).
// this time every sentence is given several times (the different possible outcomes from CRF):
// each stats with a comment e.g. "# 0 0.682380" -> you should store the sentence
// number and the confidence score somewhere; then read the words but we are interested only in the ones which have been predicted as part of an expression:
// the last two columns give 1) "gold" answer (the correct answer from train data) and 2) predicted answer (prediction from CRF).
//		we mostly need the last one (the other i'm not sure yet), and what we need is the words which have this label in the sentence (column 2).
//				we might need other info from other columns later but we will see later. maybe the function should return a vector of "sentences",
//						but with each sentence contains all the possibilities (maybe inside another vector),
//		 and each such "object" contains the expressions labelled by the CRF (in theory there can be several, dont know if this actually happens).
//		 realized that actually we need both columns: 1) the gold answer 2) the predicted answer;
//		 but the gold answer will be the same for all the possibilities given by the CRF for the sentence.
//
//		 not for myself: also i'm not sure what happens about the gold standard column when using the test data.



class annotatedcorpus {
public:
  annotatedcorpus(string file, int useLemma, int withPOS);

	void prnt_outcomes (void);
	vector<annotatedsentence> &getAnnotatedSentences();
	int nbSentences();
	annotatedsentence *getSentence(int no);
	vector<vector<double>> computeFeatures(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean, bool trainingMode);


//vector<annotated_corpus> pssble_sntncs;// we cant use the same type of itself here. the solution is to use pointer but semantically in our case is not correct alternative.

private:

	vector <annotatedsentence> sentences;



};

#endif /* ANNOTATEDCORPUS_H_ */
