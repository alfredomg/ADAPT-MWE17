/*
 * test.cpp
 *
 *  Created on: 14 Jan 2017
 *      Author: ash
 */




#include <iostream>
#include <string>
#include <vector>
#include "Corpus.h"
#include "annotatedcorpus.h"
using namespace std;

// ./test train.bio.tst.predict.test

int main( int argc, char * argv[]){


	if(argc==2){
		annotatedcorpus c(argv[1]);
	  //		Corpus c(argv[1]);
		//c.prnt_snts();
		//cout << c.c_wordfrequency("que") << endl;

	  //    c.prnt_freq();
	}else{
		cerr << "usage exactly 1 argument" << endl;
	}

}
