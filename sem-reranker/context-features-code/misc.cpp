/*
 *
 * context-main.cpp
 *
 */


#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <unordered_map>
#include "Corpus.h"
#include "Context.h"
#include "misc.h"
#include "annotatedcorpus.h"
#include "ContextCollection.h"
#include "math.h"

using namespace std;



void usage(string mainArgs, unordered_map<string,string> options) {
	cout <<  endl;
	cout << "Arguments: [options] " << mainArgs << endl;
	cout <<  endl;
	cout << "  Caution: we assume the following format:" <<  endl;
	cout << "    ref corpus: <token no> <token> [<POS tag> <lemma> ...]"  <<  endl;
	cout << "    input MWE data: <token no> <token> <POS tag> <lemma> <col5> <col6> <col7> <gold> <prediction>"  <<  endl;
	cout << endl;
	cout << "  Options:" << endl;
	cout << "    -h: print this help message" << endl;
	cout << "    -m <min freq>: specify min freq for context words; default=" << options["minFreq"] << endl;
	cout << "    -n <n most freq>: specify number of most common words to discard for context" << endl;
	cout << "        words; default=" << options["removeNMostFreq"] << endl;
	cout << "    -w <half window size>: specify number of words on the left/right to take into" << endl;
	cout << "       account for the context; 0 = full sentence; default=" <<  options["halfWindowSize"] << endl;
	cout << "    -c <0|1> normalize context words by sentence; default: " << options["normalizeContext"] << endl;
	cout << "    -s <sim measure id> value can be: jaccard, minmax, cosine, cosineIDF. Default="<< options["simMeasureId"] << endl;
	cout << "    -p <min confidence>; min confidence level for computing the features;" << endl;
	cout << "       default: " << options["minConfidence"] << endl;
	cout << "    -o <0|1> use mean only instead of min/mean/max for features (cases with multiple" << endl;
	cout << "       expressions in a sentence and multiple words in an expressions). default: " << options["featsOnlyMean"] << endl;
	cout << "    -l <0|1>: use lemma instead of token. Caution: this is possible only if the" <<endl;
	cout << "       lemmas are available in the reference corpus. default: " <<options["useLemma"] << endl;
	cout << "    -a <0|1>: choose between either to include the expression words (1) or exclude" << endl;
	cout << "       them inside the window (0). Default: "<<options["windowIncludeWordsINexpr"] << endl;
	cout << "    -b <0|1>: choose between either to include multiple occurrences of the words (1) or" << endl;
	cout << "       only including single occurrence of the word(position) (0). Default: "<<options["windowMultipleOccurrencesPosition"] << endl;
	cout << "    -C <0|1> use CRF confidence as feature (1) or not (0). default: " <<options["featConfidence"] << endl;
	cout << "    -g <0|1><0|1><0|1> three <0|1> values representing whether the following groups" <<endl;
	cout << "       of features should be used, respectively: single word, expr minus word," <<endl;
	cout << "       external exprs. Default: " <<options["featGroups"] << endl;
	cout << "    -t <0|1><0|1> two <0|1> values representing whether the following types of features"<< endl;
	cout << "       are used, respectively: frequency features, semantic similarity features. Default: " <<options["featTypes"] << endl;
	cout << "    -f <0|1><0|1><0|1><0|1> four <0|1> values representing whether the following standard" << endl;
	cout << "       features should be used, respectively: nbExpressions, nbWords, freqExprRefCorpus," <<endl;
	cout << "       nbOtherExprsAlternatives. Default: " <<options["featStd"] << endl;
	cout <<  endl;
}


unordered_map<string,string> initDefaultOptions() {
	unordered_map<string,string> options;
	options["minFreq"] = "0";
	options["removeNMostFreq"] = "0";
	options["halfWindowSize"] = "2";
	options["normalizeContext"] = "1";
	options["simMeasureId"] = "cosine";
	options["minConfidence"] = "0.0";
	options["featsOnlyMean"] = "0";
	options["useLemma"] = "0";
	options["windowIncludeWordsINexpr"] = "0";
	options["windowMultipleOccurrencesPosition"] = "0";
	options["featConfidence"] = "1";
	options["featGroups"] = "111";
	options["featTypes"] = "11";
	options["featStd"] = "1111";
	return options;
}


void printStringStringMap(unordered_map<string,string> m) {
	for (auto it : m) {
		cout << it.first << ":" << it.second << endl;
	}
}


vector<string> parseOptions(int argc, char * argv[], unordered_map<string,string> &options, string usageStr, int minArgs, int maxArgs) {
	int argIndex=1;
	int printUsage = 0;
	// as long as arg is an option starting with '-' but not '--' (standard end of options)

	//    cerr << "argc=" <<argc<<" argIndex="<<argIndex<<"\n";
	while ((argIndex < argc) && (argv[argIndex][0] == '-') && (strlen(argv[argIndex]) == 2) && (argv[argIndex][1] != '-')) {
		//  cerr << "DEBUG: argIndex="<<argIndex<<" ; arg  ="<< argv[argIndex] << "\n";
		switch (argv[argIndex][1]) { // depending on char just after '-' (we don't accept long options with more than one char)
		case 'm':
			if (++argIndex < argc) {
				options["minFreq"] = argv[argIndex];
			} else {
				cerr << "Error: option '-m' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'n':
			if (++argIndex < argc) {
				options["removeNMostFreq"] = argv[argIndex];
			} else {
				cerr << "Error: option '-n' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'w':
			if (++argIndex < argc) {
				options["halfWindowSize"] = argv[argIndex];
			} else {
				cerr << "Error: option '-w' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'c':
			if (++argIndex < argc) {
				options["normalizeContext"] = argv[argIndex];
			} else {
				cerr << "Error: option '-c' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 's':
			if (++argIndex < argc) {
				options["simMeasureId"] = argv[argIndex];
			} else {
				cerr << "Error: option '-s' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'p':
			if (++argIndex < argc) {
				options["minConfidence"] = argv[argIndex];
			} else {
				cerr << "Error: option '-p' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'o':
			if (++argIndex < argc) {
				options["featsOnlyMean"] = argv[argIndex];
			} else {
				cerr << "Error: option '-o' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'l':
			if (++argIndex < argc) {
				options["useLemma"] = argv[argIndex];
			} else {
				cerr << "Error: option '-l' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'h':
			printUsage = 1;
			break;
		case 'a':
			if (++argIndex < argc) {
				options["windowIncludeWordsINexpr"] = argv[argIndex];
			} else {
				cerr << "Error: option '-a' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'b':
			if (++argIndex < argc) {
				options["windowMultipleOccurrencesPosition"] = argv[argIndex];
			} else {
				cerr << "Error: option '-b' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'C':
			if (++argIndex < argc) {
				options["featConfidence"] = argv[argIndex];
			} else {
				cerr << "Error: option '-C' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'g':
			if (++argIndex < argc) {
				options["featGroups"] = argv[argIndex];
			} else {
				cerr << "Error: option '-g' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 't':
			if (++argIndex < argc) {
				options["featTypes"] = argv[argIndex];
			} else {
				cerr << "Error: option '-t' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;
		case 'f':
			if (++argIndex < argc) {
				options["featStd"] = argv[argIndex];
			} else {
				cerr << "Error: option '-f' requires argument but reached end of args\n";
				printUsage = 1;
			}
			break;



		default:
			cerr << "Error: unrecognized option '" << argv[argIndex][1] << "'\n";
			printUsage = 1;
		}

		argIndex++;
	}

	if ((argIndex < argc) && (argv[argIndex][0] == '-') && (strlen(argv[argIndex]) == 2) && (argv[argIndex][1] == '-')) {
		argIndex++;
	}

	if (!printUsage && (argIndex+1 >= argc)) {
		cerr << "Error: missing arguments\n";
		printUsage = 1;
	}

	if ((argc-argIndex<minArgs) || (argc-argIndex>maxArgs)) {
		cerr << "Error: wrong number of arguments (min=" << minArgs << ",max="<<maxArgs<<")\n";
		printUsage = 1;
	}

	if (printUsage) {
		usage(usageStr, options);
		exit(1);
	}

	// from here we know that we have the right arguments (normally!!)
	vector<string> args;
	args.push_back(argv[argIndex++]);
	while (argIndex < argc) {
		args.push_back(argv[argIndex]);
		argIndex++;
	}
	return args;
}



void printStringVector(const vector<string> &v) {
	cout << "[";
	if (v.size()>0) {
		cout << v[0];
		for (int i=1; i<v.size(); i++) {
			cout << ";" << v[i];
		}
	}
	cout << "]";
}


void printDoubleVector(vector<double> &v) {
	cout << "[ ";
	if (v.size()>0) {
		cout << v[0];
		for (int i=1; i<v.size(); i++) {
			cout << " ; " << v[i];
		}
	}
	cout << " ]";
}


void writeArff(string filename, vector<string> featuresNames, vector<vector<double>> features) {

	//ofstream myfile;
	//myfile.open (filename);
	//myfile << "Writing this to a file.\n";
	//myfile.close();
	cerr << "writeArff: not implemented, temporary output to STDOUT"<<endl;
	for (string name:featuresNames) {
		cout << name << "\t";
	}
	cerr << endl;
	for (vector<double> instance: features) {
		for (double val : instance) {
			cout << val << "\t";
		}
		cout << endl;
	}

}


vector<double> vectorStats(vector<double> values) {
	return vectorStats(values, 0);
}

// returns [ min, mean, max ]                                                                                                                                                                                 
// NaN values if empty                                                                                                                                                                                        
vector<double> vectorStats(vector<double> values, int onlyMean) {
	if (values.empty()) {
		cerr << "Error: cannot compute stats on empty vector\n";
		exit(1);
	} else {
		if (onlyMean) {
			vector<double> res(1);
			for (int i=0; i<values.size(); i++) {
				res[0] += values[i];
			}
			res[0] /= values.size(); // mean
			return res;
		} else {
			vector<double> res(3);// size of 3: there will be the minimum the mean and the maximum
			res[0] = values[0];// minimum
			res[1] = values[0];// mean
			res[2] = values[0];// maximum
			for (int i=1; i<values.size(); i++) {
				if (values[i]<res[0]) {
					res[0] =values[i];
				}
				if (values[i]>res[2]) {
					res[2] =values[i];
				}
				res[1] += values[i];
			}
			res[1] /= values.size(); // mean
			return res;
		}
	}
}


vector<double> vectorStatsByColumn(vector<vector<double>> values) {
	return vectorStatsByColumn(values, 0);

}

vector<double> vectorStatsByColumn(vector<vector<double>> values, int onlyMean) {
	if (values.empty()) {
		cerr<< "Error: cannot compute stats by col with zero rows\n";
		exit(1);
	} else {
		if (onlyMean) {
			vector<double> res(values[0].size());
			for (int row=0; row<values.size(); row++) {
				if (values[row].size() != values[0].size()) {
					cerr<< "Error: rows don't have the same size.\n";
					exit(1);
				}
				for (int col=0; col<values[0].size(); col++) {
					res[col] += values[row][col];
				}
			}
			for (int col=0; col<values[0].size(); col++) {
				res[col] /= values.size(); // mean
			}
			return res;
		} else {
			vector<double> res(3 * values[0].size());
			for (int col=0; col<values[0].size(); col++) {
				res[0*values[0].size()+col] = values[0][col];
				res[1*values[0].size()+col] = values[0][col];
				res[2*values[0].size()+col] = values[0][col];
			}
			for (int row=1; row<values.size(); row++) {
				if (values[row].size() != values[0].size()) {
					cerr<< "Error: rows don't have the same size.\n";
					exit(1);
				}
				for (int col=0; col<values[0].size(); col++) {
					if (values[row][col] < res[0*values[0].size()+col]) {
						res[0*values[0].size()+col] = values[row][col];
					}
					if (values[row][col] > res[2*values[0].size()+col]) {
						res[2*values[0].size()+col] = values[row][col];
					}
					res[1*values[0].size()+col] += values[row][col];
				}
			}
			for (int col=0; col<values[0].size(); col++) {
				res[1*values[0].size()+col] /=  values.size(); // mean
			}
			return res;
		}
	}
}


vector<string> vectorStatsByColumnNames(vector<string> names, int onlyMean) {
	vector<string> res;
	if (!onlyMean) {
		for (int i=0; i<names.size(); i++) {
			res.push_back(names[i]+"MIN");
		}
	}
	for (int i=0; i<names.size(); i++) {
		res.push_back(names[i]+"MEAN");
	}
	if (!onlyMean) {
		for (int i=0; i<names.size(); i++) {
			res.push_back(names[i]+"MAX");
		}
	}
	return res;
}


// to concatenate one single expression.
string joinStrings(vector<string> v, string sep) {
	string res;
	if (v.size()>0) {
		res = v[0];
		for (int i=1; i<v.size(); i++) {
			res += sep;
			res += v[i];
		}
	}
	return res;
}


void checkBoolStringParam(string name, string p, int nbChars) {
  if (p.size() != nbChars) {
    cerr << "Error: invalid size for parameter "<<name<<" (value '"<<p<<"'), must be "<<nbChars<<" \n";
    exit(1);
  }
  for (int i=0;i<nbChars;i++) {
    if ((p[i] != '0') && (p[i] != '1')) {
      cerr << "Error: invalid value for boolean array parameter "<<name<<" (value '"<<p<<"'), each character must be 0 or 1 \n";
      exit(1);
    }
  }
}



void contextFeaturesMain(int argc, char *argv[], bool trainMode) {

	string usageStr = "<corpus file> <annotated CRF output> <output features file>";


	//cout << "Im the right one" << '\n';

	unordered_map<string, string> options = initDefaultOptions();

	// parsing options
	vector<string> mainArgs = parseOptions(argc, argv, options, usageStr, 3, 3);

	string corpusFile = mainArgs[0];
	string inputFromCRF = mainArgs[1];
	string outputFile =  mainArgs[2];

	printStringStringMap(options);

	checkBoolStringParam("featGroups", options["featGroups"], 3);
	checkBoolStringParam("featTypes", options["featTypes"], 2);
	checkBoolStringParam("featStd", options["featStd"], 4);

	//@@@
	//cout<< corpusFile;

	// build reference corpus
	Corpus corpus(corpusFile, stoi(options["useLemma"]), stoi(options["minFreq"]), stoi(options["removeNMostFreq"]), options["simMeasureId"]);

	// read input from CRF with multiple solutions for each sentence
	annotatedcorpus data(inputFromCRF, stoi(options["useLemma"]), 0);

	// gather potential expressions from input
	ContextCollection allExpressions(data, options, &corpus, trainMode);
	vector<vector<double>> feats = data.computeFeatures(stoi(options["featConfidence"]), options["featGroups"], options["featTypes"], options["featStd"], stoi(options["featsOnlyMean"]), trainMode);
	vector<string> featNames = annotSentenceFeatNames(stoi(options["featConfidence"]), options["featGroups"], options["featTypes"], options["featStd"], stoi(options["featsOnlyMean"]));

	//printStringVector(featNames);
	writeDoubleFeats(outputFile, feats, featNames);
	//writeDoubleFeats(outputFile, feats);

}


// default feat names
void writeDoubleFeats(string file, vector<vector<double>> features) {
	if (features.size()==0) {
		cerr << "Error: no instances at all\n";
		exit(1);
	}
	int nbFeats = features[0].size();
	vector<string> res(nbFeats);
	char buff[100];
	for (int i=0; i< nbFeats; i++) {
		snprintf(buff, sizeof(buff), "feat%03d", i);
		res[i] = buff;
	}
	res[nbFeats-1] = "answer";
	writeDoubleFeats(file, features, res);
}



void writeDoubleFeats(string file, vector<vector<double>> features, vector<string> featNames) {
	ofstream f;
	f.open(file);
	if (featNames.size()==0) {
		cerr << "Error: cannot have 0 features\n";
		exit(1);
	}
	f << featNames[0];
	for (int i=1; i< featNames.size(); i++) {
		f << "\t" << featNames[i];
	}
	f << "\n";
	for (int row=0; row<features.size(); row++) {
		if (features[row].size() != featNames.size()) {
			cerr << "Error: different number of features in row " << row << ": "<< features[row].size()<<" vs. features names number = " << featNames.size() << "\n";
			exit(1);
		}
		f << features[row][0];
		for (int i=1; i< featNames.size(); i++) {
			double v = features[row][i];
			if (isnan(v) || isinf(v)) {
				f << "\tNA";
			} else {
				f << "\t" << v;
			}
		}
		f << "\n";
	}

	f.close();
}




/* contraction cases appear at least in French in the following way (example in English for the sake of clarity):
 *
 * ...
 * 6-7 isn't
 * 6 is
 * 7 not
 * ...
 *
 * i.e. the contracted version appears with a range of tokens e.g. 6-7, then the sequence of individual tokens show the expanded version.
 * If using tokens we want to keep only the contracted version, since that's how the words would appear in the reference corpus.
 * But on the contrary if using lemmas we want to keep the expanded version, since the contracted version doesn't have a lemma
 */
int ignoreTokenWrtContractions(string tokenNoStr, int useLemma, int *ignoreTokenRangeEnd) {

	int posSep = tokenNoStr.find('-');
	int contracted = 0;
	int expanded = 0;
	if (posSep != string::npos) { // found range "X-Y"
		//    cerr << "DEBUG: contracted; range= " << tokenNoStr <<endl;
		*ignoreTokenRangeEnd =  stoi(tokenNoStr.substr(posSep+1));
		contracted = 1;
	} else {
		int tokenNo = stoi(tokenNoStr);
		if ((*ignoreTokenRangeEnd != -1) && (tokenNo <= *ignoreTokenRangeEnd)) { // if token is part of an  expanded contraction
			//      cerr << "DEBUG: expanded; no= " << tokenNoStr <<endl;
			expanded = 1;
			if (tokenNo == *ignoreTokenRangeEnd) { // last token inside range:
				*ignoreTokenRangeEnd = -1;         // reset range
			}
		} // otherwise it's just a regular token
	}
	// if lemma then ignore the contracted form and use the expanded words
	return (!useLemma && expanded) || (useLemma && contracted);

}


// this function returns all the columes in the string line.

vector<string> getColValues(string line) {
	vector<string> res;
	int start=0;
	int nextTab = line.find("\t",start);
	while (nextTab != string::npos) {
		res.push_back(line.substr(start, nextTab-start));
		start = nextTab+1;
		nextTab = line.find("\t",start);
		//    cerr <<"DEBUG getColValues: '"<<res.back()<<"'\n";
	}
	// last element
	res.push_back(line.substr(start));
	return res;
}
