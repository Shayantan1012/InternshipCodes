#ifndef OUTLIERDETECTOR_H
#define OUTLIERDETECTOR_H
#include <vector>
using namespace std;
class OutlierDetector
{
private:
    vector<vector<double>> points;
    int k;
    double euclideanDistance(const vector<double>& a, const vector<double>& b);
    vector<vector<int>> getKNearestNeighbors();
public:
    OutlierDetector(vector<vector<double>> data, int kNeighbors);
    vector<double> computeLOFScores();
    vector<vector<double>> filterOutliers(
        const vector<double>& lofScores,
        double threshold,
        vector<vector<double>>& noisePoints,
        vector<int>& noiseIndices
    );
};
#endif