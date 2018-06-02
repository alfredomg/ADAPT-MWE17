/*
 * Corpus.h
 *
 *  Created on: 14 Jan 2017
 *      Author: ash
 */

#ifndef CORPUS_H_
#define CORPUS_H_



#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include "Sentence.h"
#include <unordered_map>
#include <unordered_set>
using namespace std;


class Corpus {// here in this class corpus we are not dealing with sentences at all.

public:
  
  Corpus(string file, int useLemma, int min_frreq, int num_mostfrreq, string simM);

	void prnt_snts(void);

	int c_wordfrequency(string word);

	void prnt_freq(void);

	Sentence &getSentence(int i); // return the reference of the sentence i in c_snt.
	void contextvocabularywords(void);
    bool isinCorpus(string word);

	bool isContextVocabWord(string &word);
	int nbSentences();
	double getIDF(string word);
	void initDocFreq();
	string &getSimMeasureId();
	unordered_set<int> sentencesWhichContain(const vector<string> &words); //we have the intersection of the words


private:
	vector<Sentence> c_snt;

	unordered_map<string,int> freq;
	unordered_map<string,int> docFreq;// we add the word only once at the sentence level.
	unordered_set<string> contextVocabWords;

	//finding the sentences which contains the expression.
	unordered_map<string,unordered_set<int>> sentencesByWord; // probably not very well designed with freq, docFreq etc.// inverted index
	// it contains all the sentnces for each word.
	// the index of the sentences
	//the word of the sentneces.

	
	int min_freq, num_mostfreq;
	string simMeasureId;
};

#endif /* CORPUS_H_ */
