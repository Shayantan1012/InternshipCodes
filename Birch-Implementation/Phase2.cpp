#include "Phase2.h"

#include <iostream>


using namespace std;



Phase2::Phase2(int minimumPoints)
{

    minPoints = minimumPoints;

}



vector<CF> Phase2::condense(CFNode* root)
{

    vector<CF> microClusters;


    extract(root,microClusters);



    vector<CF> filtered;



    for(auto &cf:microClusters)
    {


        if(cf.getN() >= minPoints)
        {

            filtered.push_back(cf);

        }

        else
        {

            cout<<"Removing small CF N="
                <<cf.getN()
                <<endl;

        }


    }



    cout<<"\nPhase 2 Remaining CFs = "
        <<filtered.size()
        <<endl;


    return filtered;

}




void Phase2::extract(
    CFNode* node,
    vector<CF>& result
)
{


    if(node->isLeaf())
    {


        vector<CF>& entries =
            node->getEntries();



        for(auto &cf:entries)
        {

            result.push_back(cf);

        }


    }

    else
    {


        vector<CFNode*>& children =
            node->getChildren();



        for(auto child:children)
        {

            extract(child,result);

        }

    }


}