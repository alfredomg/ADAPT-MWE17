/*
 * Corpus.cpp
 *
 *  Created on: 14 Jan 2017
 *      Author: ash
 */


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <locale>
#include "Corpus.h"
#include "misc.h"
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <math.h>
using namespace std;




Corpus::Corpus(string file, int useLemma, int min_frreq, int num_mostfrreq, string simM) {

	int colTokenOrLemma =  (useLemma) ? 4 : 2; // the number of the column which contains either the word or the lemma
	min_freq = min_frreq;
	num_mostfreq = num_mostfrreq;
	simMeasureId = simM;

	string line;
	ifstream filestream;
	int ignoreTokenRangeEnd = -1;
	locale loc;// depends on the language.
	int j, i;
	Sentence snt;
	c_snt.clear();
	freq.clear();
	docFreq.clear();
	contextVocabWords.clear();
	int sentNo=0;

	filestream.open(file.c_str());


	if(filestream.is_open()) {

		cout << "Reading corpus: '"<<file<<"'... "<<endl;

		while(getline(filestream, line)){
			//cout << "size=" << c_snt.size() <<". sent: " << line  << endl;

			if(!line.empty() && isdigit(line[0],loc)){


				vector<string> columns = getColValues(line);

				if (columns.size() < colTokenOrLemma) { // to check if its the right file// this function returns all
					cerr << "Error: wrong number of columns\n";
					exit(3);
				}


				int ignoreToken =  ignoreTokenWrtContractions(columns[0], useLemma, &ignoreTokenRangeEnd);

				if (!ignoreToken) {
					string word = columns[colTokenOrLemma-1];
					//	  cerr << "DEBUG Corpus: adding word '"<<word <<"' to sentence." << endl;

					snt.addwords(word);

					// this tells us if word is already inside the map:
					unordered_map<string,int>::iterator found = freq.find(word);// the function find works with the key.
					if (found == freq.end()) { // not found = new word
						freq[word] = 1;


					} else { // word already in the nap: increment count
						// freq[word]++;  // same but probably a bit less efficient
						found->second++; // the value.
					}
					// below used to be: sentencesByWord[word].insert(sentNo); // hopefully this works whether it's a new word or not
					// changed it hoping it would solve seg fault, but it didn't
					//so  i'm not sure but the new version is probably safer
					auto found2 = sentencesByWord.find(word);
					if (found2 == sentencesByWord.end()) {
						unordered_set<int> newSentSet;
						newSentSet.insert(sentNo);
						sentencesByWord[word] = newSentSet;
					} else {
						found2->second.insert(sentNo);
					}

					// not effient
					// to count the freq of the words in the seen sentences.
					//				if(!c_snt.empty()){
					//					int counts=0;
					//					for(int i =0; i<= c_snt.size()-1; i++){
					//
					//						//c_snt[i].prnt_wrd();
					//						vector <string> rslt = c_snt[i].getwords();
					//						//	cout << rslt.size() << endl;
					//						for(int j =0; j<= rslt.size()-1; j++){
					//							if(word == rslt[j]){
					//							  freq[word]= counts++;
					//							}
					//						}
					//						//					snt.addwords(word);
					//
					//					}
					//				}else{
					//					freq[word] = 1;
					//				}
				}
			} else { // empty line in EUROPAL, which means that we just finished reading the current sentence snt (and we re going to start a new one)

				if (!snt.getwords().empty()) {
					//				snt.prnt_wrd();
					c_snt.push_back(snt);
					sentNo++;
					// i need to

					vector<string> &sentWords = snt.getwords();
					unordered_set<string> sntFreq(sentWords.begin(), sentWords.end());
					for (const auto& word: sntFreq) {
						//					for (auto it = sntFreq.begin(); it != sntFreq.end(); ++it) {
						unordered_map<string, int>::iterator found2 = docFreq.find(word);
						if (found2 == docFreq.end()) {
							docFreq[word] = 1; // appears for the first time.
						} else {
							docFreq[word]++; // if it appears again in a different sentence.
						}

					}

					//					vector<string> &sentWords = snt.getwords();
					//					unordered_set<string> sntFreq();
					//
					//					for(int i=0; i<=sentWords.size()-1; i++){
					//
					//
					//						unordered_set<string>::iterator found = sntFreq.find(sentWords[i]);
					//							if(found == sntFreq.end()){
					//
					//							sntFreq.insert(c_snt[i]);
					//							unordered_map<string, int>::iterator found2 = docFreq.find(sentWords[i]);
					//								if (found2 == docFreq.end()) {
					//									docFreq[sentWords[i]] = 1;
					//								} else {
					//									docFreq[sentWords[i]]++;
					//								}
					//							}
					//
					//					}

					sntFreq.clear();


					snt.clear(); // clear the vector inside the object // we start a new vector for the empty line.
					//snt();// we need destructor.
				}

			}
			// BAD! dont uncomment please!
			//			c_snt.push_back(snt);

		} // while loop (end of file)



		if (!snt.getwords().empty()) { // store last sentence!!
			//				snt.prnt_wrd();
			c_snt.push_back(snt);
			sentNo++;
			// i need to

			vector<string> &sentWords = snt.getwords();
			unordered_set<string> sntFreq(sentWords.begin(), sentWords.end());
			for (const auto& word: sntFreq) {
				//					for (auto it = sntFreq.begin(); it != sntFreq.end(); ++it) {
				unordered_map<string, int>::iterator found2 = docFreq.find(word);
				if (found2 == docFreq.end()) {
					docFreq[word] = 1; // appears for the first time.
				} else {
					docFreq[word]++; // if it appears again in a different sentence.
				}

			}
		}

		if (sentNo != c_snt.size()) { // sanity check
			cerr << "BUG! sentNo does not match the number of sentences read in the corpus!" << endl;
			exit(1);
		}



	}else{
		cerr <<"there is an error opening the file! file='" << file <<"'" << endl;
	}

	contextvocabularywords();

}

void Corpus::prnt_snts(void){

	for(int i = 0; i <c_snt.size(); i++){

		c_snt[i].prnt_wrd();
	}
}



Sentence &Corpus::getSentence(int i) {
	return c_snt[i];
}


// function used only in Corpus::contextvocabularywords below: compares 2 pairs word+freq based on their frequency
bool compareWordFreqPairs(pair<string, int> pair1,pair<string, int> pair2) {
	return (pair1.second<pair2.second); // the sign shows the ascending order.
}

void Corpus::contextvocabularywords(){

	int maxFreq = INT_MAX;  // default max freq = very high (so no word is filtered out)
	if (num_mostfreq>0) {
		// we create this vecor in order to sort the input from the map inside this vector
		// 1) create a vector which contains elements which are pairs word+frequency copied from the map freq
		vector<pair<string, int>> wordFreqPairs(freq.begin(), freq.end());
		// 2) sort this vector by frequency (	using the function compareWordFreqPairs above)
		sort(wordFreqPairs.begin(), wordFreqPairs.end(), compareWordFreqPairs); // INTERSTEING.

		// 3) done by ascending order then we get the value for the threshold.
		int limit = wordFreqPairs.size() - num_mostfreq; //the position in the sorted vector.
		maxFreq = wordFreqPairs[limit].second;//like a threshold.
	}
       // 4) to remove the things that we are going to remove from the map according to step number 3.
	for (auto it = freq.begin(); it != freq.end(); ++it){ // iterate over all the words
		if((this->min_freq <= it->second) && (maxFreq >= it->second)) { // if the frequency of the word is at least min_freq, add the word to the set
			contextVocabWords.insert(it->first);

		}
	}
}







//std::vector<std::pair<char, int>> elems(table.begin(), table.end());
//std::sort(elems.begin(), elems.end(), comp);





void Corpus::prnt_freq(){

	for ( auto it = freq.begin(); it != freq.end(); ++it){

		cout << " " << it->first << ":" << it->second;
		cout<< endl;

	}

}



int Corpus::c_wordfrequency(string word){

	int count=0;

	for(int i=0; i<=c_snt.size()-1; i++){
		if(!this->c_snt.empty()){


			vector <string> result = c_snt[i].getwords();
			for(int j =0; j<=result.size()-1; j++){
				if(result [j] == word){

					count ++;
				}
			}
		}
	}
	return count;
}




//to look if the word is in the set or not.
bool Corpus::isContextVocabWord(string& word) {
	unordered_set<string>::const_iterator found = contextVocabWords.find(word);
	//		cerr << "checking word: "<< word<<endl;
	if(found == contextVocabWords.end()){
		//		cerr << "false"<<endl;
		return false;
	}else{
		//		cerr << "true"<<endl;
		return true;
	}
}





int Corpus::nbSentences() {
	return c_snt.size();
}




//double Corpus::getIDF(string word) {
//
//	unordered_map<string, int>::iterator found = docFreq.find(word);
//
//
//int total =c_snt.size()+1;
//
//
//if(found == docFreq.end()){
//
//	int dom =1;
//
//	return log(total/dom);
//
//}else{
//
//
//	int wrdfre = found->second;
//
//	return log(total/wrdfre);
//}
//
//}


// returns the Inverse Document Frequency weight
double Corpus::getIDF(string word) {
	unordered_map<string, int>::const_iterator it = docFreq.find(word);
	int total = c_snt.size() +1;
	if (it == docFreq.end()) {
		return log(total/1 );
	} else {
		return log(total / (1+it->second) );
	}
}



bool Corpus::isinCorpus(string word){

	unordered_map<string, int>::const_iterator found = docFreq.find(word);
	if(found == docFreq.end()){
		return false;
	}else{
		return true;
	}

}

string &Corpus::getSimMeasureId() {
	return simMeasureId;
}



//we want all the sentences which contains EXACTLY all words in this vector &words its ok in the right order but not as unit.
//for example words vec contains cat and so we look at all the sentences that contains cat and to build up the context vector for them.
unordered_set<int> Corpus::sentencesWhichContain(const vector<string> &words) { // (const vector<string> &words) == (vector<string> expr) ; contains this one expression.
	if (words.size() == 0) {
		cerr << "Error: 0 words in vector in function sentencesWhichContain\n";
		exit(1);
	}

	unordered_set<int> sentences;
	auto found = sentencesByWord.find(words[0]);// sentencesBy (IN THE CORPUS) word contains the index of the sentences  which contains this specif word// the fist word in the expression?.
	if (found == sentencesByWord.end()) {// if we didn't find it it means the word is not in the corpus in the first place.
		return sentences; // empty set
	} else {
		sentences = found->second; // the second part unordered_map<string,unordered_set<int>> sentencesByWord is the set of int..
	}                            // what are we taking the snetence index for this word of the expression.
	for (int i=1; i<words.size(); i++) {
		auto found = sentencesByWord.find(words[i]);
		if ((found == sentencesByWord.end()) || (sentences.size()==0)) {
			unordered_set<int> empty;
			return empty; // empty set
		} else { // intersecting
			// remark: there was a segfault due to erasing element while iterating over "sentences", which invalidates the iterator
			// the solution was to use the iterator returned by erase function (points to next element)
			unordered_set<int> sentencesCurrentWord = found->second;// comes from i. e.g. cat
			unordered_set<int>::iterator it = sentences.begin();// first word rain word [0]
			while (it != sentences.end()) {
				//      for (int sentNo : sentences) {
				auto found2 = sentencesCurrentWord.find(*it); // find sentNo that belongs to [rain] so the next step is to match (find) it in the second coloum [cat]
				if (found2 == sentencesCurrentWord.end()) { // sentNo does not belong to both
					it = sentences.erase(it); // returns next iterator in sentences // how???
				} else {
					it++;
				}
			}
		}
	}

	return sentences;
}


