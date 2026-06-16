#include "CF.h"


CF::CF(int dimension)
{

    N = 0;

    LS.resize(dimension,0.0);

    SS.resize(dimension,0.0);

}



void CF::addPoint(vector<double> point)
{

    N++;


    for(int i=0;i<point.size();i++)
    {

        LS[i] += point[i];

        SS[i] += point[i]*point[i];

    }

}



vector<double> CF::getCentroid()
{

    vector<double> centroid;


    for(int i=0;i<LS.size();i++)
    {

        centroid.push_back(LS[i]/N);

    }


    return centroid;

}



int CF::getN()
{

    return N;

}