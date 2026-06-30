#include "OutlierDetector.h"
#include <cmath>
#include <algorithm>
using namespace std;
OutlierDetector::OutlierDetector(vector<vector<double>> data, int kNeighbors)
{
    points = data;
    k = kNeighbors;
}
double OutlierDetector::euclideanDistance(const vector<double>& a, const vector<double>& b)
{
    double sum = 0.0;
    for(size_t i = 0; i < a.size(); i++)
    {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}
vector<vector<int>> OutlierDetector::getKNearestNeighbors()
{
    int n = points.size();
    vector<vector<int>> neighbors(n);
    for(int i = 0; i < n; i++)
    {
        vector<pair<double,int>> dists;
        for(int j = 0; j < n; j++)
        {
            if(i == j) continue;
            double d = euclideanDistance(points[i], points[j]);
            dists.push_back({d, j});
        }
        sort(dists.begin(), dists.end());
        for(int m = 0; m < k && m < (int)dists.size(); m++)
        {
            neighbors[i].push_back(dists[m].second);
        }
    }
    return neighbors;
}
vector<double> OutlierDetector::computeLOFScores()
{
    int n = points.size();
    vector<vector<int>> kNeighbors = getKNearestNeighbors();
    vector<double> kDistance(n);
    for(int i = 0; i < n; i++)
    {
        int lastNeighbor = kNeighbors[i].back();
        kDistance[i] = euclideanDistance(points[i], points[lastNeighbor]);
    }
    vector<double> lrd(n);
    for(int i = 0; i < n; i++)
    {
        double sumReach = 0.0;
        for(int j : kNeighbors[i])
        {
            double d = euclideanDistance(points[i], points[j]);
            double reachDist = max(d, kDistance[j]);
            sumReach += reachDist;
        }
        lrd[i] = (sumReach == 0.0) ? 0.0 : (double)kNeighbors[i].size() / sumReach;
    }
    vector<double> lof(n);
    for(int i = 0; i < n; i++)
    {
        double sumRatio = 0.0;
        for(int j : kNeighbors[i])
        {
            if(lrd[i] == 0.0) sumRatio += 0.0;
            else sumRatio += lrd[j] / lrd[i];
        }
        lof[i] = (kNeighbors[i].empty()) ? 1.0 : sumRatio / kNeighbors[i].size();
    }
    return lof;
}
vector<vector<double>> OutlierDetector::filterOutliers(
    const vector<double>& lofScores,
    double threshold,
    vector<vector<double>>& noisePoints,
    vector<int>& noiseIndices)
{
    vector<vector<double>> cleanPoints;
    for(size_t i = 0; i < points.size(); i++)
    {
        if(lofScores[i] > threshold)
        {
            noisePoints.push_back(points[i]);
            noiseIndices.push_back(i);
        }
        else
        {
            cleanPoints.push_back(points[i]);
        }
    }
    return cleanPoints;
}