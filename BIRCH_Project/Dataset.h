#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <string>

using namespace std;


class Dataset
{

private:

    vector<vector<double>> data;
    vector<string> groundTruth;


public:

    bool loadCSV(string filename);
    bool loadLabeledCSV(string filename);


    void normalize();


    vector<vector<double>> getData();
    vector<string> getGroundTruth();
    bool hasGroundTruth();


};


#endif
