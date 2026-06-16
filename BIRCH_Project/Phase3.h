#ifndef PHASE3_H
#define PHASE3_H


#include "CF.h"

#include<vector>


using namespace std;



class Phase3
{


public:


    Phase3(int clusters);


    vector<int> cluster(
        vector<CF>& microClusters
    );



private:


    int k;



    double distance(
        vector<double> a,
        vector<double> b
    );



};



#endif