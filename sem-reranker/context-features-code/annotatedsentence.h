/*
 * annotatedsentence.h
 *
 *  Created on: 26 Jan 2017
 *      Author: ash
 */

#ifndef ANNOTATEDSENTENCE_H_
#define ANNOTATEDSENTENCE_H_

#include <vector>
#include <string>
#include "outcomes.h"

using namespace std;

class annotatedsentence {
public:
  annotatedsentence();
  virtual ~annotatedsentence();

  bool isEmpty();
  void addOutcome(int id, double confi);
  void addWord(string word, string gold, string predict);
  void clear();
  outcomes *getGold();
  vector<outcomes> &getOutcomes();
  outcomes *getOutcomes(int i);
  int getNbOutcomes();
  vector<vector<double>> computeFeatures(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean,bool trainingMode,int sentNo);
  


private:
	vector<outcomes> pssble_sntncs;
	outcomes gold;


};

// not class function
vector<string> annotSentenceFeatNames(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean);
   

#endif /* ANNOTATEDSENTENCE_H_ */
