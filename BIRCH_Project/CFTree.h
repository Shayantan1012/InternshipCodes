#ifndef CFTREE_H
#define CFTREE_H


#include "CFNode.h"
#include <vector>


using namespace std;


class CFTree
{

private:

    CFNode* root;

    double threshold;

    int branchingFactor;


    void splitRoot();


    void printNode(CFNode* node);


    int countCFs(CFNode* node);



public:

    CFTree(double T,int B);

    void insert(vector<double> point);

    void printTree();

    void printClusters();


    // Phase 2 access

    CFNode* getRoot();

    vector<CF> getLeafCFs();

};


#endif