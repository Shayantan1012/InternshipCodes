#include <iostream>
#include <algorithm>
#include "Dataset.h"
#include "OutlierDetector.h"
#include "CFTree.h"
#include "GlobalClustering.h"
#include "Phase2.h"
#include "Phase3.h"
#include "Output.h"
#include "Metrics.h"
using namespace std;
int main()
{
    Dataset dataset;
    if(!dataset.loadCSV("two_emitter_pdw_dataset.csv"))
    {
        return 1;
    }
    dataset.normalize();
    vector<vector<double>> data = dataset.getData();
    cout<<"Rows: "<<data.size()<<endl;
    cout<<"Features: "<<data[0].size()<<endl;

    // ============================
    // STEP 1: OUTLIER DETECTION (LOF) - BEFORE CLUSTERING
    // ============================
    cout<<"\n===== OUTLIER DETECTION (LOF) =====\n";
    int kNeighbors = 10;
    double lofThreshold = 1.5;
    OutlierDetector detector(data, kNeighbors);
    vector<double> lofScores = detector.computeLOFScores();
    vector<vector<double>> noisePoints;
    vector<int> noiseIndices;
    vector<vector<double>> cleanData = detector.filterOutliers(lofScores, lofThreshold, noisePoints, noiseIndices);
    cout<<"Total points: "<<data.size()<<endl;
    cout<<"Noise points removed: "<<noisePoints.size()<<endl;
    cout<<"Clean points remaining: "<<cleanData.size()<<endl;

    vector<vector<double>> dataForClustering = cleanData;

    double threshold = 0.5;
    int branchingFactor = 5;
    CFTree tree(threshold, branchingFactor);

    // ============================
    // PHASE 1: CF TREE CONSTRUCTION (only on clean points)
    // ============================
    for(auto point : dataForClustering)
    {
        tree.insert(point);
    }
    tree.printTree();
    tree.printClusters();
    vector<CF> leafCFs = tree.getLeafCFs();
    GlobalClustering::printCFs(leafCFs);

    // ============================
    // PHASE 2: CONDENSE CF TREE
    // ============================
    cout<<"\n===== PHASE 2 =====\n";
    Phase2 phase2(5);
    vector<CF> cleanCFs = phase2.condense(tree.getRoot());
    cout<<"CFs after condensation: "<<cleanCFs.size()<<endl;

    // ============================
    // PHASE 3: GLOBAL CLUSTERING
    // ============================
    cout<<"\n===== PHASE 3 =====\n";
    Phase3 phase3(2);
    vector<int> labels = phase3.cluster(cleanCFs);
    for(size_t i=0;i<labels.size();i++)
    {
        cout<<"Micro Cluster "<<i<<" belongs to Cluster "<<labels[i]<<endl;
    }

    // ============================
    // FINAL DATA POINT CLUSTER ASSIGNMENT
    // ============================
    cout<<"\n===== FINAL DATA LABELING =====\n";
    vector<int> clusterLabelsForClean = Output::assignClusters(dataForClustering, cleanCFs, labels);

    vector<int> finalLabels(data.size(), -1);
    int cleanPointer = 0;
    for(size_t i = 0; i < data.size(); i++)
    {
        bool isNoise = find(noiseIndices.begin(), noiseIndices.end(), (int)i) != noiseIndices.end();
        if(!isNoise)
        {
            finalLabels[i] = clusterLabelsForClean[cleanPointer];
            cleanPointer++;
        }
    }

    // ============================
    // LOAD GROUND TRUTH + MAP PREDICTED NAMES
    // ============================
    dataset.loadGroundTruth("two_emitter_pdw_labeled.csv");
    vector<string> predictedNames = Output::mapClustersToEmitterNames(dataset.getRawData(), finalLabels);

    // ============================
    // SAVE FINAL LABELED OUTPUT CSV
    // ============================
    Output::saveLabeledCSV(
        "birch_results.csv",
        dataset.getRawData(),
        dataset.getGroundTruth(),
        predictedNames
    );
    cout<<"\nResult saved to birch_results.csv\n";

    // ============================
    // VALIDATION METRICS
    // ============================
    vector<string> groundTruth = dataset.getGroundTruth();

    Metrics::evaluateOutlierDetection(groundTruth, noiseIndices, data.size());
    Metrics::evaluateClustering(groundTruth, predictedNames);
    Metrics::evaluateCombined(groundTruth, predictedNames);

    return 0;
}
