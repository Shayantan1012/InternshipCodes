#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include <string>
using namespace std;
class Dataset
{
private:
    vector<vector<double>> data;
    vector<vector<double>> rawData;
    vector<string> groundTruth;
public:
    bool loadCSV(string filename);
    void normalize();
    vector<vector<double>> getData();
    vector<vector<double>> getRawData();
    bool loadGroundTruth(string filename);
    vector<string> getGroundTruth();
};
#endif