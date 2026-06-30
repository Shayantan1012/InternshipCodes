#ifndef CFNODE_H
#define CFNODE_H

#include <vector>
#include "CF.h"

using namespace std;


class CFNode
{

private:

    bool leaf;

    int maxEntries;


    vector<CF> entries;


    vector<CFNode*> children;


public:


    CFNode(int B, bool isLeaf);



    bool isLeaf();


    void setLeaf(bool value);



    int getSize();



    bool isOverflow();



    void addCF(CF cf);



    void addChild(CFNode* child);



    vector<CF>& getEntries();



    vector<CFNode*>& getChildren();



    CF& getCF(int index);



    int findClosestCF(vector<double> point);



    void clearEntries();


};


#endif