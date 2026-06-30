#ifndef OUTPUT_H
#define OUTPUT_H
#include <vector>
#include <string>
#include "CF.h"
using namespace std;
class Output
{
public:
    static vector<int> assignClusters(
        vector<vector<double>> data,
        vector<CF> finalCFs,
        vector<int> cfLabels
    );
    static void saveCSV(
        string filename,
        vector<vector<double>> data,
        vector<int> labels
    );
    static vector<string> mapClustersToEmitterNames(
        vector<vector<double>> rawData,
        vector<int> finalLabels
    );
    static void saveLabeledCSV(
        string filename,
        vector<vector<double>> rawData,
        vector<string> groundTruth,
        vector<string> predictedLabels
    );
private:
    static double distance(
        vector<double> a,
        vector<double> b
    );
};
#endif