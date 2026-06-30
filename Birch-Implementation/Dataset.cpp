#include "Dataset.h"
#include <fstream>
#include <sstream>
#include <iostream>
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
    rawData = data;
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
            if(data[i][j]<minVal[j]) minVal[j]=data[i][j];
            if(data[i][j]>maxVal[j]) maxVal[j]=data[i][j];
        }
    }
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<cols;j++)
        {
            data[i][j]=(data[i][j]-minVal[j])/(maxVal[j]-minVal[j]);
        }
    }
}
vector<vector<double>> Dataset::getData()
{
    return data;
}
vector<vector<double>> Dataset::getRawData()
{
    return rawData;
}
bool Dataset::loadGroundTruth(string filename)
{
    ifstream file(filename);
    if(!file.is_open())
    {
        cout<<"Cannot open ground truth file"<<endl;
        return false;
    }
    string line;
    getline(file, line);
    while(getline(file, line))
    {
        stringstream ss(line);
        string value;
        string lastValue;
        while(getline(ss, value, ','))
        {
            lastValue = value;
        }
        if(!lastValue.empty() && lastValue.back() == '\r')
        {
            lastValue.pop_back();
        }
        groundTruth.push_back(lastValue);
    }
    file.close();
    return true;
}
vector<string> Dataset::getGroundTruth()
{
    return groundTruth;
}
