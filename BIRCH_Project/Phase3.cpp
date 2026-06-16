#include "Phase3.h"

#include <cmath>
#include <iostream>


using namespace std;



Phase3::Phase3(int clusters)
{

    k=clusters;

}



double Phase3::distance(
    vector<double> a,
    vector<double> b
)
{

    double sum=0;


    for(int i=0;i<a.size();i++)
    {

        sum += pow(a[i]-b[i],2);

    }


    return sqrt(sum);

}



vector<int> Phase3::cluster(
    vector<CF>& microClusters
)
{


    vector<vector<double>> centers;



    for(int i=0;i<k;i++)
    {

        centers.push_back(
            microClusters[i].getCentroid()
        );

    }



    vector<int> labels(
        microClusters.size(),
        -1
    );



    bool change=true;



    while(change)
    {


        change=false;



        // Assignment


        for(int i=0;i<microClusters.size();i++)
        {


            double bestDistance=999999;


            int bestCluster=0;



            vector<double> point =
            microClusters[i].getCentroid();



            for(int c=0;c<k;c++)
            {


                double d=
                distance(
                    point,
                    centers[c]
                );



                if(d<bestDistance)
                {

                    bestDistance=d;

                    bestCluster=c;

                }


            }



            if(labels[i]!=bestCluster)
            {

                labels[i]=bestCluster;

                change=true;

            }


        }





        // Update centers


        vector<vector<double>> newCenters(
            k,
            vector<double>(
                microClusters[0]
                .getCentroid()
                .size(),
                0
            )
        );


        vector<int> count(k,0);



        for(int i=0;i<microClusters.size();i++)
        {


            int id=labels[i];


            vector<double> c =
            microClusters[i]
            .getCentroid();



            for(int j=0;j<c.size();j++)
            {

                newCenters[id][j]+=c[j];

            }


            count[id]++;


        }



        for(int c=0;c<k;c++)
        {


            if(count[c]>0)
            {


                for(int j=0;j<newCenters[c].size();j++)
                {

                    newCenters[c][j]/=count[c];

                }


            }


        }



        centers=newCenters;


    }



    return labels;


}