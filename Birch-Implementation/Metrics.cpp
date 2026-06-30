#include "Metrics.h"
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

namespace
{
struct ClassificationScore
{
    double precision;
    double recall;
    double f1;
};

ClassificationScore calculateClassScore(
    const vector<string>& groundTruth,
    const vector<string>& predictedLabels,
    const string& className,
    bool excludeGroundTruthNoise)
{
    int TP = 0;
    int FP = 0;
    int FN = 0;

    for(size_t i = 0; i < groundTruth.size(); i++)
    {
        if(excludeGroundTruthNoise && groundTruth[i] == "Noise") continue;

        bool actualPositive = (groundTruth[i] == className);
        bool predictedPositive = (predictedLabels[i] == className);

        if(actualPositive && predictedPositive) TP++;
        else if(!actualPositive && predictedPositive) FP++;
        else if(actualPositive && !predictedPositive) FN++;
    }

    ClassificationScore score;
    score.precision = (TP + FP > 0) ? (double)TP / (TP + FP) : 0;
    score.recall = (TP + FN > 0) ? (double)TP / (TP + FN) : 0;
    score.f1 = (score.precision + score.recall > 0)
        ? 2 * score.precision * score.recall / (score.precision + score.recall)
        : 0;
    return score;
}

void printClassificationMetrics(
    ostream& output,
    const vector<string>& groundTruth,
    const vector<string>& predictedLabels,
    const vector<string>& classNames,
    bool excludeGroundTruthNoise)
{
    double precisionSum = 0;
    double recallSum = 0;
    double f1Sum = 0;

    for(const string& className : classNames)
    {
        ClassificationScore score = calculateClassScore(
            groundTruth,
            predictedLabels,
            className,
            excludeGroundTruthNoise
        );

        output<<className
            <<" Precision="<<score.precision
            <<" Recall="<<score.recall
            <<" F1-Score="<<score.f1
            <<endl;

        precisionSum += score.precision;
        recallSum += score.recall;
        f1Sum += score.f1;
    }

    double classCount = classNames.size();
    output<<"Macro Precision = "<<precisionSum / classCount<<endl;
    output<<"Macro Recall = "<<recallSum / classCount<<endl;
    output<<"Macro F1-Score = "<<f1Sum / classCount<<endl;
}
}

void Metrics::evaluateOutlierDetection(vector<string> groundTruth, vector<int> noiseIndices, int totalPoints)
{
    int TP = 0, FP = 0, FN = 0, TN = 0;
    vector<bool> predictedNoise(totalPoints, false);
    for(int idx : noiseIndices)
    {
        predictedNoise[idx] = true;
    }
    for(int i = 0; i < totalPoints; i++)
    {
        bool actualNoise = (groundTruth[i] == "Noise");
        bool predNoise = predictedNoise[i];
        if(actualNoise && predNoise) TP++;
        else if(!actualNoise && predNoise) FP++;
        else if(actualNoise && !predNoise) FN++;
        else TN++;
    }
    double precision = (TP + FP > 0) ? (double)TP / (TP + FP) : 0;
    double recall = (TP + FN > 0) ? (double)TP / (TP + FN) : 0;
    double f1 = (precision + recall > 0) ? 2 * precision * recall / (precision + recall) : 0;
    cout<<"\n===== OUTLIER DETECTION METRICS (LOF) =====\n";
    cout<<"TP="<<TP<<" FP="<<FP<<" FN="<<FN<<" TN="<<TN<<endl;
    cout<<"Precision = "<<precision<<endl;
    cout<<"Recall = "<<recall<<endl;
    cout<<"F1-Score = "<<f1<<endl;

    ofstream report("../metrics/birch_validation_report.txt");
    if(report.is_open())
    {
        report<<"BIRCH IMPLEMENTATION VALIDATION REPORT\n";
        report<<"======================================\n\n";
        report<<"===== OUTLIER DETECTION METRICS (LOF) =====\n";
        report<<"TP="<<TP<<" FP="<<FP<<" FN="<<FN<<" TN="<<TN<<endl;
        report<<"Precision = "<<precision<<endl;
        report<<"Recall = "<<recall<<endl;
        report<<"F1-Score = "<<f1<<endl;
    }
}
void Metrics::evaluateClustering(vector<string> groundTruth, vector<string> predictedLabels)
{
    int correct = 0;
    int total = 0;
    for(size_t i = 0; i < groundTruth.size(); i++)
    {
        if(groundTruth[i] == "Noise") continue;
        total++;
        if(groundTruth[i] == predictedLabels[i]) correct++;
    }
    double accuracy = (total > 0) ? (double)correct / total : 0;
    cout<<"\n===== CLUSTERING METRICS (BIRCH only, Emitter_1 vs Emitter_2) =====\n";
    cout<<"Correctly clustered = "<<correct<<" / "<<total<<endl;
    cout<<"Clustering Accuracy = "<<accuracy<<endl;
    printClassificationMetrics(
        cout,
        groundTruth,
        predictedLabels,
        {"Emitter_1", "Emitter_2"},
        true
    );

    ofstream report("../metrics/birch_validation_report.txt", ios::app);
    if(report.is_open())
    {
        report<<"\n===== CLUSTERING METRICS (BIRCH only, Emitter_1 vs Emitter_2) =====\n";
        report<<"Correctly clustered = "<<correct<<" / "<<total<<endl;
        report<<"Clustering Accuracy = "<<accuracy<<endl;
        printClassificationMetrics(
            report,
            groundTruth,
            predictedLabels,
            {"Emitter_1", "Emitter_2"},
            true
        );
    }
}
void Metrics::evaluateCombined(vector<string> groundTruth, vector<string> predictedLabels)
{
    int correct = 0;
    int total = groundTruth.size();
    for(int i = 0; i < total; i++)
    {
        if(groundTruth[i] == predictedLabels[i]) correct++;
    }
    double accuracy = (total > 0) ? (double)correct / total : 0;
    cout<<"\n===== COMBINED PIPELINE METRICS (Outlier + Clustering) =====\n";
    cout<<"Correctly labeled = "<<correct<<" / "<<total<<endl;
    cout<<"Overall Accuracy = "<<accuracy<<endl;
    printClassificationMetrics(
        cout,
        groundTruth,
        predictedLabels,
        {"Emitter_1", "Emitter_2", "Noise"},
        false
    );

    ofstream report("../metrics/birch_validation_report.txt", ios::app);
    if(report.is_open())
    {
        report<<"\n===== COMBINED PIPELINE METRICS (Outlier + Clustering) =====\n";
        report<<"Correctly labeled = "<<correct<<" / "<<total<<endl;
        report<<"Overall Accuracy = "<<accuracy<<endl;
        printClassificationMetrics(
            report,
            groundTruth,
            predictedLabels,
            {"Emitter_1", "Emitter_2", "Noise"},
            false
        );
        cout<<"\nValidation report saved to ../metrics/birch_validation_report.txt\n";
    }
}
