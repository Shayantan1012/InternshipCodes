#include "GlobalClustering.h"

#include <iostream>


using namespace std;



void GlobalClustering::printCFs(vector<CF> cfs)
{

    cout<<"\n===== Phase 2 Input CFs =====\n";


    for(size_t i=0;i<cfs.size();i++)
    {

        cout<<"\nCF "
            <<i
            <<endl;


        cout<<"Weight N = "
            <<cfs[i].getN()
            <<endl;


        cout<<"Centroid: ";


        vector<double> centroid =
            cfs[i].getCentroid();


        for(double x:centroid)
        {
            cout<<x<<" ";
        }


        cout<<endl;

    }


}
