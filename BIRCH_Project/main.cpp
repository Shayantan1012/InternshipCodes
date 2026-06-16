#include <iostream>
#include <algorithm>
#include <map>
#include <set>

#include "Dataset.h"
#include "CFTree.h"
#include "GlobalClustering.h"
#include "Phase2.h"
#include "Phase3.h"
#include "Output.h"
using namespace std;


double bestAccuracy(
    vector<int> predicted,
    vector<string> truth
)
{
    set<int> clusterSet(predicted.begin(), predicted.end());
    set<string> truthSet(truth.begin(), truth.end());

    vector<int> clusters(clusterSet.begin(), clusterSet.end());
    vector<string> truthLabels(truthSet.begin(), truthSet.end());

    sort(truthLabels.begin(), truthLabels.end());

    int bestCorrect = 0;

    do
    {
        map<int,string> mapping;

        for(size_t i=0;i<clusters.size() && i<truthLabels.size();i++)
        {
            mapping[clusters[i]] = truthLabels[i];
        }

        int correct = 0;

        for(size_t i=0;i<predicted.size();i++)
        {
            if(mapping[predicted[i]] == truth[i])
            {
                correct++;
            }
        }

        if(correct > bestCorrect)
        {
            bestCorrect = correct;
        }

    } while(next_permutation(truthLabels.begin(), truthLabels.end()));

    return 100.0 * bestCorrect / truth.size();
}


int main()
{

    const string inputFile = "../two_emitter_pdw_labeled.csv";
    const bool useGroundTruth = true;
    const string outputFile = "birch_results.csv";
    const double threshold = 0.5;
    const int branchingFactor = 5;
    const int phase2MinPoints = 5;
    const int finalClusters = 2;

    Dataset dataset;


    bool loaded = useGroundTruth
        ? dataset.loadLabeledCSV(inputFile)
        : dataset.loadCSV(inputFile);


    if(!loaded)
    {
        return 1;
    }



    dataset.normalize();



    vector<vector<double>> data =
        dataset.getData();



    cout<<"Rows: "
        <<data.size()
        <<endl;


    cout<<"Features: "
        <<data[0].size()
        <<endl;


    CFTree tree(
        threshold,
        branchingFactor
    );



    // ============================
    // PHASE 1
    // CF TREE CONSTRUCTION
    // ============================


    for(auto point:data)
    {
        tree.insert(point);
    }



    tree.printTree();

    tree.printClusters();



    vector<CF> leafCFs =
        tree.getLeafCFs();



    GlobalClustering::printCFs(leafCFs);




    // ============================
    // PHASE 2
    // CONDENSE CF TREE
    // ============================


    cout<<"\n===== PHASE 2 =====\n";



    Phase2 phase2(phase2MinPoints);



    vector<CF> cleanCFs =
        phase2.condense(
            tree.getRoot()
        );



    cout<<"CFs after condensation: "
        <<cleanCFs.size()
        <<endl;




    // ============================
    // PHASE 3
    // GLOBAL CLUSTERING
    // ============================


    cout<<"\n===== PHASE 3 =====\n";



    Phase3 phase3(finalClusters);



    vector<int> labels =
        phase3.cluster(
            cleanCFs
        );



    for(size_t i=0;i<labels.size();i++)
    {

        cout<<"Micro Cluster "
            <<i
            <<" belongs to Cluster "
            <<labels[i]
            <<endl;

    }
// =====================================
// FINAL DATA POINT CLUSTER ASSIGNMENT
// =====================================


cout<<"\n===== FINAL DATA LABELING =====\n";



vector<int> finalLabels =
    Output::assignClusters(
        data,
        cleanCFs,
        labels
    );



for(size_t i=0;i<finalLabels.size();i++)
{

    cout<<"Point "
        <<i
        <<" -> Cluster "
        <<finalLabels[i]
        <<endl;

}



// =====================================
// SAVE OUTPUT CSV
// =====================================


if(dataset.hasGroundTruth())
{
    vector<string> groundTruth =
        dataset.getGroundTruth();

    double accuracy =
        bestAccuracy(
            finalLabels,
            groundTruth
        );

    cout<<"\n===== GROUND TRUTH VALIDATION =====\n";
    cout<<"Best mapped accuracy: "
        <<accuracy
        <<"%"
        <<endl;

    Output::saveCSV(

        outputFile,

        data,

        finalLabels,

        groundTruth

    );
}
else
{
    Output::saveCSV(

        outputFile,

        data,

        finalLabels

    );
}


cout<<"\nResult saved to "
    <<outputFile
    <<"\n";


    return 0;

}
