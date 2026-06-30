#include "Output.h"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;
double Output::distance(vector<double> a, vector<double> b)
{
    double sum = 0;
    for(size_t i=0;i<a.size();i++)
    {
        sum += pow(a[i]-b[i],2);
    }
    return sqrt(sum);
}
vector<int> Output::assignClusters(vector<vector<double>> data, vector<CF> finalCFs, vector<int> cfLabels)
{
    vector<int> labels;
    for(auto point:data)
    {
        double minDistance = 999999;
        int bestCluster = -1;
        for(size_t i=0;i<finalCFs.size();i++)
        {
            vector<double> centroid = finalCFs[i].getCentroid();
            double d = distance(point, centroid);
            if(d < minDistance)
            {
                minDistance = d;
                bestCluster = cfLabels[i];
            }
        }
        labels.push_back(bestCluster);
    }
    return labels;
}
void Output::saveCSV(string filename, vector<vector<double>> data, vector<int> labels)
{
    ofstream file(filename);
    if(!file.is_open())
    {
        cout<<"Cannot create output file\n";
        return;
    }
    file<<"Feature1,Feature2,Feature3,Feature4,Feature5,Cluster\n";
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
vector<string> Output::mapClustersToEmitterNames(vector<vector<double>> rawData, vector<int> finalLabels)
{
    double sumFreq0 = 0, sumFreq1 = 0;
    int count0 = 0, count1 = 0;
    for(size_t i = 0; i < finalLabels.size(); i++)
    {
        if(finalLabels[i] == 0)
        {
            sumFreq0 += rawData[i][1];
            count0++;
        }
        else if(finalLabels[i] == 1)
        {
            sumFreq1 += rawData[i][1];
            count1++;
        }
    }
    double avgFreq0 = (count0 > 0) ? sumFreq0 / count0 : 0;
    string name0, name1;
    if(abs(avgFreq0 - 5900) < abs(avgFreq0 - 6100))
    {
        name0 = "Emitter_1";
        name1 = "Emitter_2";
    }
    else
    {
        name0 = "Emitter_2";
        name1 = "Emitter_1";
    }
    vector<string> predicted;
    for(size_t i = 0; i < finalLabels.size(); i++)
    {
        if(finalLabels[i] == -1) predicted.push_back("Noise");
        else if(finalLabels[i] == 0) predicted.push_back(name0);
        else predicted.push_back(name1);
    }
    return predicted;
}
void Output::saveLabeledCSV(string filename, vector<vector<double>> rawData, vector<string> groundTruth, vector<string> predictedLabels)
{
    ofstream file(filename);
    if(!file.is_open())
    {
        cout<<"Cannot create output file\n";
        return;
    }
    file<<"TOA_ns,Freq_MHz,PW_ns,Az_deg,El_deg,Ground_Truth,Predicted_Cluster\n";
    for(size_t i = 0; i < rawData.size(); i++)
    {
        for(size_t j = 0; j < rawData[i].size(); j++)
        {
            file<<rawData[i][j]<<",";
        }
        file<<groundTruth[i]<<","<<predictedLabels[i]<<"\n";
    }
    file.close();
    cout<<"Labeled CSV file created successfully\n";
}
