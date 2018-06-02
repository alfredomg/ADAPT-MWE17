/*
 * MWE.h
 *
 *  Created on: 14 Jan 2017
 *      Author: ash
 */

#ifndef MWE_H_
#define MWE_H_



#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Sentence {

public:
  Sentence(void);
  void addwords(string tkn);
  void prnt_wrd(void);
  void clear(void);
  vector <string>& getwords(void);// to get all the tokens for a sentence. & for reference
  vector<int> findExpression(vector<string> &expr, int maxDist);

private:
  vector <string> tokens;
};

#endif /* MWE_H_ */
