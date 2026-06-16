#ifndef OUTPUT_H
#define OUTPUT_H


#include <vector>
#include <string>

#include "CF.h"


using namespace std;



class Output
{


public:


    // Assign every original point
    // to nearest BIRCH cluster

    static vector<int> assignClusters(

        vector<vector<double>> data,

        vector<CF> finalCFs,

        vector<int> cfLabels

    );



    // Save final result

    static void saveCSV(

        string filename,

        vector<vector<double>> data,

        vector<int> labels

    );


    static void saveCSV(

        string filename,

        vector<vector<double>> data,

        vector<int> labels,

        vector<string> groundTruth

    );



private:


    static double distance(

        vector<double> a,

        vector<double> b

    );


};



#endif
