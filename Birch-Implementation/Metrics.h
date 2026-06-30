#ifndef METRICS_H
#define METRICS_H
#include <vector>
#include <string>
using namespace std;
class Metrics
{
public:
    static void evaluateOutlierDetection(
        vector<string> groundTruth,
        vector<int> noiseIndices,
        int totalPoints
    );
    static void evaluateClustering(
        vector<string> groundTruth,
        vector<string> predictedLabels
    );
    static void evaluateCombined(
        vector<string> groundTruth,
        vector<string> predictedLabels
    );
};
#endif