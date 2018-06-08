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
#include <math.h>
#include <limits>

using namespace std;



void usage(string mainArgs, unordered_map<string,string> options);
unordered_map<string,string> initDefaultOptions();
void printStringStringMap(unordered_map<string,string> m);
vector<string> parseOptions(int argc, char * argv[], unordered_map<string,string> &options, string usageStr, int minArgs, int maxArgs);
void printStringVector(const vector<string> &v);
void printDoubleVector(vector<double> &v);
void writeArff(string filename, vector<string> featuresNames, vector<vector<double>> features);
vector<double> vectorStats(vector<double> values) ;
vector<double> vectorStats(vector<double> values, int onlyMean) ;
vector<double> vectorStatsByColumn(vector<vector<double>> values) ;
vector<double> vectorStatsByColumn(vector<vector<double>> values, int onlyMean) ;
vector<string> vectorStatsByColumnNames(vector<string> names, int onlyMean) ;
string joinStrings(vector<string> v, string sep);
void contextFeaturesMain(int argc, char *argv[], bool trainMode);

void writeDoubleFeats(string file, vector<vector<double>> features);
void writeDoubleFeats(string file, vector<vector<double>> features, vector<string> featNames);

int ignoreTokenWrtContractions(string tokenNoStr, int useLemma, int *ignoreTokenRangeEnd);
vector<string> getColValues(string line);
