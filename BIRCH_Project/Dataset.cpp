#include "Dataset.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>


using namespace std;



bool Dataset::loadCSV(string filename)
{

    ifstream file(filename);


    if(!file.is_open())
    {
        cout<<"Cannot open file"<<endl;
        return false;
    }


    string line;


    // remove header
    getline(file,line);



    while(getline(file,line))
    {

        vector<double> row;


        stringstream ss(line);


        string value;


        while(getline(ss,value,','))
        {

            row.push_back(stod(value));

        }


        data.push_back(row);

    }


    file.close();


    return true;

}




bool Dataset::loadLabeledCSV(string filename)
{

    ifstream file(filename);


    if(!file.is_open())
    {
        cout<<"Cannot open file"<<endl;
        return false;
    }


    string line;


    // remove header: Row_Number,features...,Ground_Truth
    getline(file,line);


    while(getline(file,line))
    {

        vector<string> cells;

        stringstream ss(line);

        string value;


        while(getline(ss,value,','))
        {
            cells.push_back(value);
        }


        if(cells.size() < 3)
        {
            continue;
        }


        vector<double> row;


        for(int i=1;i<cells.size()-1;i++)
        {
            row.push_back(stod(cells[i]));
        }


        data.push_back(row);
        groundTruth.push_back(cells.back());

    }


    file.close();


    return true;

}




void Dataset::normalize()
{

    int rows=data.size();

    int cols=data[0].size();



    vector<double> minVal(cols);

    vector<double> maxVal(cols);



    for(int j=0;j<cols;j++)
    {

        minVal[j]=data[0][j];

        maxVal[j]=data[0][j];

    }



    for(int i=0;i<rows;i++)
    {

        for(int j=0;j<cols;j++)
        {

            if(data[i][j]<minVal[j])
                minVal[j]=data[i][j];


            if(data[i][j]>maxVal[j])
                maxVal[j]=data[i][j];

        }

    }



    for(int i=0;i<rows;i++)
    {

        for(int j=0;j<cols;j++)
        {

            if(maxVal[j] == minVal[j])
            {
                data[i][j] = 0.0;
            }
            else
            {
                data[i][j]=
                (data[i][j]-minVal[j])/
                (maxVal[j]-minVal[j]);
            }

        }

    }


}




vector<vector<double>> Dataset::getData()
{

    return data;

}




vector<string> Dataset::getGroundTruth()
{

    return groundTruth;

}




bool Dataset::hasGroundTruth()
{

    return !groundTruth.empty();

}
