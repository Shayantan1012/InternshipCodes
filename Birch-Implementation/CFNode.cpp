#include "CFNode.h"
#include <cmath>
#include <limits>
using namespace std;
CFNode::CFNode(int B, bool isLeaf)
{
    maxEntries = B;
    leaf = isLeaf;
}
bool CFNode::isLeaf() { return leaf; }
void CFNode::setLeaf(bool value) { leaf = value; }
int CFNode::getSize()
{
    if(leaf) return static_cast<int>(entries.size());
    else return static_cast<int>(children.size());
}
bool CFNode::isOverflow()
{
    if(leaf) return entries.size() > static_cast<size_t>(maxEntries);
    else return children.size() > static_cast<size_t>(maxEntries);
}
void CFNode::addCF(CF cf) { entries.push_back(cf); }
void CFNode::addChild(CFNode* child) { children.push_back(child); }
vector<CF>& CFNode::getEntries() { return entries; }
vector<CFNode*>& CFNode::getChildren() { return children; }
CF& CFNode::getCF(int index) { return entries[index]; }
void CFNode::clearEntries() { entries.clear(); }
int CFNode::findClosestCF(vector<double> point)
{
    double minimum = numeric_limits<double>::max();
    int closest = 0;
    for(size_t i=0;i<entries.size();i++)
    {
        vector<double> centroid = entries[i].getCentroid();
        double distance = 0;
        for(size_t j=0;j<point.size();j++)
        {
            distance += pow(point[j]-centroid[j],2);
        }
        distance = sqrt(distance);
        if(distance < minimum)
        {
            minimum = distance;
            closest = static_cast<int>(i);
        }
    }
    return closest;
}
