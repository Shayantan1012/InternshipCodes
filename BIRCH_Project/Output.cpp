#include "Output.h"

#include <iostream>
#include <fstream>
#include <cmath>


using namespace std;



// -----------------------------------------
// Euclidean distance between two vectors
// -----------------------------------------

double Output::distance(

    vector<double> a,

    vector<double> b

)
{

    double sum = 0;


    for(size_t i=0;i<a.size();i++)
    {

        sum += pow(a[i]-b[i],2);

    }


    return sqrt(sum);

}



// -----------------------------------------
// Assign every original data point
// to nearest final CF centroid
// -----------------------------------------

vector<int> Output::assignClusters(

    vector<vector<double>> data,

    vector<CF> finalCFs,

    vector<int> cfLabels

)
{


    vector<int> labels;



    for(auto point:data)
    {


        double minDistance = 999999;


        int bestCluster = -1;



        // Compare with every final CF

        for(size_t i=0;i<finalCFs.size();i++)
        {


            vector<double> centroid =

                finalCFs[i].getCentroid();



            double d = distance(

                point,

                centroid

            );



            if(d < minDistance)

            {

                minDistance = d;


                // take Phase 3 cluster id

                bestCluster = cfLabels[i];

            }

        }



        labels.push_back(bestCluster);


    }



    return labels;


}



// -----------------------------------------
// Save final clustering result
// -----------------------------------------

void Output::saveCSV(

    string filename,

    vector<vector<double>> data,

    vector<int> labels

)
{


    ofstream file(filename);



    if(!file.is_open())

    {

        cout<<"Cannot create output file\n";

        return;

    }



    // Header

    file<<"Feature1,"
        <<"Feature2,"
        <<"Feature3,"
        <<"Feature4,"
        <<"Feature5,"
        <<"Cluster\n";




    for(size_t i=0;i<data.size();i++)

    {


        for(size_t j=0;j<data[i].size();j++)

        {

            file<<data[i][j]<<",";

        }



        file<<labels[i];


        file<<"\n";


    }



    file.close();



    cout<<"CSV file created successfully\n";


}




void Output::saveCSV(

    string filename,

    vector<vector<double>> data,

    vector<int> labels,

    vector<string> groundTruth

)
{


    ofstream file(filename);



    if(!file.is_open())

    {

        cout<<"Cannot create output file\n";

        return;

    }



    file<<"Feature1,"
        <<"Feature2,"
        <<"Feature3,"
        <<"Feature4,"
        <<"Feature5,"
        <<"Cluster,"
        <<"Ground_Truth\n";




    for(size_t i=0;i<data.size();i++)

    {


        for(size_t j=0;j<data[i].size();j++)

        {

            file<<data[i][j]<<",";

        }



        file<<labels[i]<<","
            <<groundTruth[i];


        file<<"\n";


    }



    file.close();



    cout<<"CSV file created successfully\n";


}
