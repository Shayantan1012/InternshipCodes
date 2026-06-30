#include "CFTree.h"

#include <iostream>
#include <cmath>
#include <limits>
using namespace std;
CFTree::CFTree(double T, int B)
{
    threshold = T;
    branchingFactor = B;
    root = new CFNode(B,true);

}
// Insert point into BIRCH tree
void CFTree::insert(vector<double> point)
{
    // If root is leaf

    if(root->isLeaf())
    {
        // First CF

        if(root->getSize()==0)
        {

            CF cf(point.size());

            cf.addPoint(point);

            root->addCF(cf);

            return;

        }

        int index =
        root->findClosestCF(point);

        CF& closest =
        root->getCF(index);
        vector<double> centroid =
        closest.getCentroid();

        double distance=0;

        for(size_t i=0;i<point.size();i++)
        {

            distance +=
            pow(point[i]-centroid[i],2);

        }

        distance=sqrt(distance);

        cout<<"Distance = "
            <<distance
            <<endl;

        if(distance <= threshold)
        {

            closest.addPoint(point);

            cout<<"Merged into existing CF"
                <<endl;

        }

        else
        {

            CF newCF(point.size());
            newCF.addPoint(point);
            root->addCF(newCF);
            cout<<"Created new CF"
                <<endl;

        }
        if(root->isOverflow())
        {
            cout<<"Node Overflow detected"
                <<endl;


            splitRoot();
        }
    }
}

// Root split

void CFTree::splitRoot()
{
    cout<<"Splitting root node..."
        <<endl;
    vector<CF>& entries =
        root->getEntries();
    if(entries.size()<2)
        return;
    int seed1=0;
    int seed2=1;
    double maxDistance=0;
    for(size_t i=0;i<entries.size();i++)
    {

        for(size_t j=i+1;j<entries.size();j++)
        {


            vector<double> c1 =
            entries[i].getCentroid();


            vector<double> c2 =
            entries[j].getCentroid();



            double d=0;



            for(size_t k=0;k<c1.size();k++)
            {

                d+=pow(c1[k]-c2[k],2);

            }


            d=sqrt(d);



            if(d>maxDistance)
            {

                maxDistance=d;

                seed1=static_cast<int>(i);

                seed2=static_cast<int>(j);

            }

        }

    }



    cout<<"Seed 1 = "
        <<seed1
        <<endl;


    cout<<"Seed 2 = "
        <<seed2
        <<endl;



    CFNode* node1 =
    new CFNode(branchingFactor,true);



    CFNode* node2 =
    new CFNode(branchingFactor,true);



    node1->addCF(entries[seed1]);

    node2->addCF(entries[seed2]);




    for(size_t i=0;i<entries.size();i++)
    {

        if(static_cast<int>(i)==seed1 || static_cast<int>(i)==seed2)
            continue;



        vector<double> c =
        entries[i].getCentroid();



        vector<double> s1 =
        entries[seed1].getCentroid();



        vector<double> s2 =
        entries[seed2].getCentroid();



        double d1=0;

        double d2=0;



        for(size_t j=0;j<c.size();j++)
        {

            d1+=pow(c[j]-s1[j],2);

            d2+=pow(c[j]-s2[j],2);

        }



        if(d1<d2)

            node1->addCF(entries[i]);

        else

            node2->addCF(entries[i]);

    }



    root->clearEntries();



    root->setLeaf(false);



    root->addChild(node1);

    root->addChild(node2);



    cout<<"Split completed"
        <<endl;


}



// Count final micro clusters

int CFTree::countCFs(CFNode* node)
{


    if(node->isLeaf())
    {

        return node->getEntries().size();

    }


    int total=0;



    for(auto child:node->getChildren())
    {

        total += countCFs(child);

    }


    return total;

}



// Print number of CFs

void CFTree::printTree()
{

    cout<<"\nNumber of CFs = "
        <<countCFs(root)
        <<endl;

}



// Print clusters

void CFTree::printClusters()
{

    cout<<"\n===== BIRCH Micro Clusters =====\n";


    printNode(root);

}



// Recursive printing

void CFTree::printNode(CFNode* node)
{

    static int globalCF = 0;


    if(node->isLeaf())
    {

        vector<CF>& cfs =
            node->getEntries();


        cout << "\n--- Leaf Node ---\n";


        for(size_t i=0;i<cfs.size();i++)
        {


            cout<<"\nMicro Cluster "
                <<globalCF++
                <<endl;


            cout<<"Number of points: "
                <<cfs[i].getN()
                <<endl;



            vector<double> centroid =
                cfs[i].getCentroid();



            cout<<"Centroid: ";


            for(double x:centroid)
            {
                cout<<x<<" ";
            }


            cout<<endl;

        }


    }

    else
    {

        cout<<"\n--- Internal Node ---\n";


        vector<CFNode*>& children =
            node->getChildren();



        for(CFNode* child:children)
        {

            printNode(child);

        }

    }

}
void collectLeafCFs(CFNode* node, vector<CF>& result)
{

    if(node->isLeaf())
    {

        vector<CF>& entries =
            node->getEntries();


        for(auto &cf : entries)
        {
            result.push_back(cf);
        }

    }

    else
    {

        vector<CFNode*>& children =
            node->getChildren();


        for(auto child : children)
        {
            collectLeafCFs(child,result);
        }

    }

}



vector<CF> CFTree::getLeafCFs()
{

    vector<CF> result;


    if(root->isLeaf())
    {

        vector<CF>& entries =
            root->getEntries();


        for(auto &cf:entries)
        {
            result.push_back(cf);
        }

    }

    else
    {

        vector<CFNode*>& children =
            root->getChildren();


        for(auto child:children)
        {

            vector<CF>& entries =
                child->getEntries();


            for(auto &cf:entries)
            {
                result.push_back(cf);
            }

        }

    }


    return result;

}



// ADD THIS BELOW


CFNode* CFTree::getRoot()
{
    return root;
}
