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
#include "misc.h"

using namespace std;




void usageP(string mainArgs, unordered_map<string,string> options) {
  cout <<  endl;
  cout << "Arguments: [options] " << mainArgs << endl;
  cout <<  endl;
  cout << "  Caution: we assume the following format:" <<  endl;
  cout << "    input MWE data: <token no> <token> <POS tag> <lemma> <col5> <col6> <col7> <gold> <prediction>"  <<  endl;
  cout << endl;
  cout << "  Options:" << endl;
  cout << "    -h: print this help message" << endl;
  cout << "    -c <min confidence>; min confidence level for computing the features;" << endl;
  cout << "       default: " << options["minConfidence"] << endl;
  cout << "    -l <0|1>: use lemma instead of token. Caution: this is possible only if the" <<endl;
  cout << "       lemmas are available in the reference corpus. default: " <<options["useLemma"] << endl;
  cout << "    -p <0|1>: with POS tags (1) or not (0). default: " <<options["withPOS"] << endl;
  cout <<  endl;
}


unordered_map<string,string> initDefaultOptionsP() {
  unordered_map<string,string> options;
  options["minConfidence"] = "0.0";
  options["useLemma"] = "0";
  options["withPOS"] = "0";
  return options;
}




vector<string> parseOptionsP(int argc, char * argv[], unordered_map<string,string> &options, string usageStr, int minArgs, int maxArgs) {
  int argIndex=1;
  int printUsage = 0;
  // as long as arg is an option starting with '-' but not '--' (standard end of options)

  //    cerr << "argc=" <<argc<<" argIndex="<<argIndex<<"\n";
  while ((argIndex < argc) && (argv[argIndex][0] == '-') && (strlen(argv[argIndex]) == 2) && (argv[argIndex][1] != '-')) {
    //  cerr << "DEBUG: argIndex="<<argIndex<<" ; arg  ="<< argv[argIndex] << "\n";
    switch (argv[argIndex][1]) { // depending on char just after '-' (we don't accept long options with more than one char)
    case 'c':
      if (++argIndex < argc) {
	options["minConfidence"] = argv[argIndex];
      } else {
	cerr << "Error: option '-c' requires argument but reached end of args\n"; 
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
    case 'p':
      if (++argIndex < argc) {
	options["withPOS"] = argv[argIndex];
      } else {
	cerr << "Error: option '-p' requires argument but reached end of args\n"; 
	printUsage = 1;
      }
      break;
    case 'h':
      printUsage = 1;
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
    usageP(usageStr, options);
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






int main(int argc, char * argv[]){

  string usageStr = "<annotated CRF output> <output file>";

  unordered_map<string, string> options = initDefaultOptionsP();

  // parsing options
  vector<string> mainArgs = parseOptionsP(argc, argv, options, usageStr, 2, 2);
  
  string inputFromCRF = mainArgs[0];
  string outputFile =  mainArgs[1];

  printStringStringMap(options);

  // read input from CRF with multiple solutions for each sentence
  annotatedcorpus data(inputFromCRF, stoi(options["useLemma"]), stoi(options["withPOS"]));


  // gather potential expressions from input
  ContextCollection allExpressions(data, options, NULL, true);

  cout << "Info: collected "<< allExpressions.size() <<" distinct expressions from the corpus" << endl;


  allExpressions.printExpressions(outputFile);
}

