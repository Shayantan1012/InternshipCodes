#ifndef PHASE2_H
#define PHASE2_H


#include "CF.h"
#include "CFNode.h"

#include <vector>


using namespace std;



class Phase2
{

public:

    Phase2(int minimumPoints);


    vector<CF> condense(CFNode* root);



private:

    int minPoints;


    void extract(
        CFNode* node,
        vector<CF>& result
    );


};



#endif