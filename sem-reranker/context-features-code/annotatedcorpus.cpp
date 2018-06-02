/*
 * annotatedcorpus.cpp
 *
 *  Created on: 25 Jan 2017
 *      Author: ash
 */

#include "annotatedcorpus.h"
#include "misc.h"

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


// update Erwan 20/2/17: columnNo is the column number where the token or lemma should be read from

annotatedcorpus::annotatedcorpus(string file, int useLemma, int withPOS) {


	string line;
	ifstream filestream;
	locale loc;
	double confidenceScore;
	annotatedsentence currentSent;
	sentences.clear();
	int ignoreTokenRangeEnd = -1;

	int colTokenOrLemma =  (useLemma) ? 4 : 2;
	int colPOS = 3;
       
	
	filestream.open(file.c_str());

    // **

	int i =0;
    //
	if(filestream.is_open()) {

	  cout << "Reading annotated data (input): '" << file<< "'... "<<endl;
		while(getline(filestream, line)){


	//	  cout << "line num: "<< i << " : ";
	//	  cout <<line << endl;
	//		i++;



			if(!line.empty() && line[0]== '#'){

         //
			//	cout << "line[0]"<<  line[0] << endl;

        //
				int currentSentId = stoi(line.substr(2,1));
		//


		//		cout << "currentSentId" << " "<< currentSentId << endl;
       //

				if(currentSentId == 0){ // true when the id is 0 at the begining  of the new set// if we are starting a new set of sentences.// at the fist time we don't need it.
				  //				  cout << "new sent "<< sentences.size()<<"\n";
					if(!currentSent.isEmpty()){ // not true the first time // if the current senetence already contains something.
						annotatedsentence copy = currentSent;
						sentences.push_back(copy);
						ignoreTokenRangeEnd = - 1; // reset range
						currentSent.clear();
						/*
						cout << "DEBUG last alternative sequence (outcome) "<<copy.getOutcomes().size()-1<<" , sequence was = ";
						copy.getOutcomes().at(copy.getOutcomes().size()-1).print();
						cout << "\n";
						cout << "DEBUG end sent, gold was = ";
						copy.getGold()->print();
						cout << "\n";
						*/
					}
				}
				string confStr = line.substr(4, string::npos);
				double conf = stod(confStr);
				//cout << "confStr="<<confStr<<";conf="<<conf<<endl;
				currentSent.addOutcome(currentSentId, conf); // each time we have the # sign.
				/*
				if (currentSentId>0) {
				  // printing the previous one because the one we just added has not been read yet
				  cout << "DEBUG end alternative sequence (outcome) "<<currentSentId-1<<" , sequence was = ";
				  currentSent.getOutcomes().at(currentSentId-1).print();
				  cout << "\n";
				}
				*/


			}




			if(!line.empty() &&  isdigit(line[0],loc)){  // regular token line

       //
			  //				cout << "line after #: " << line[0] << endl;
       //

			  vector<string> columns = getColValues(line);

			  if (columns.size() < 9) {
			    cerr << "Error: wrong number of columns\n";
			    exit(3);
			  }
			  

			  int ignoreToken =  ignoreTokenWrtContractions(columns[0], useLemma, &ignoreTokenRangeEnd);

			  if (!ignoreToken) {
			    string word = columns[colTokenOrLemma-1];
			    if (withPOS) {
			      word += "/" + columns[colPOS-1];
			    }
			    string gold = columns[7];
			    string prdctd = columns[8];
			    //			    cerr << "DEBUG: adding word '"<<word <<"' to sentence; gold="<<gold<<"; pred="<<prdctd<<endl;
			    currentSent.addWord(word, gold, prdctd);
			  }

			}
		}


		if(!currentSent.isEmpty()){ //
			sentences.push_back(currentSent);
			/*
			cout << "DEBUG last alternative sequence (outcome) "<<currentSent.getOutcomes().size()-1<<" , sequence was = ";
			currentSent.getOutcomes().at(currentSent.getOutcomes().size()-1).print();
			cout << "\n";
			cout << "DEBUG end sent, gold was = ";
			currentSent.getGold()->print();
			cout << "\n";
			*/
		}

	}


}




vector<annotatedsentence> &annotatedcorpus::getAnnotatedSentences() {
  return sentences;
}

vector<vector<double>> annotatedcorpus::computeFeatures(bool featConfidence, string featGroups, string featTypes, string featStd, bool featsOnlyMean, bool trainingMode) {
  vector<vector<double>> feats;
  cout <<"Computing features for annotated sentences..." <<endl;
  int total = sentences.size();
  for (int i=0; i< total; i++) {
    cout << "\r" << i << " / " << total;
    vector<vector<double>> t = sentences[i].computeFeatures(featConfidence, featGroups, featTypes, featStd, featsOnlyMean, trainingMode, i);
    feats.insert(feats.end(), t.begin(), t.end());
  }
  cout << endl;
  return feats;
}

int annotatedcorpus::nbSentences() {
  return sentences.size();
}


annotatedsentence *annotatedcorpus::getSentence(int no) {
  return &sentences[no];
}

