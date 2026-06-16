#ifndef CF_H
#define CF_H

#include <vector>

using namespace std;


class CF
{

private:

    int N;

    vector<double> LS;

    vector<double> SS;


public:

    CF(int dimension);


    void addPoint(vector<double> point);


    vector<double> getCentroid();


    int getN();


};


#endif