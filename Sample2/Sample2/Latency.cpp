#include "stdafx.h"

std::default_random_engine* LatNode::generator;
std::uniform_int_distribution<int>* LatNode::distr;
std::uniform_int_distribution<int>* LatNode::distr2;
LatNode::LatNodeStatCons LatNode::stat_cons;

LatNode::LatNode()
{
    n = (*distr)(*generator);
}


double distMS(LatNode& a, LatNode& b)
{
    return a.distToMS(b);
}

uint64_t dist(LatNode& a, LatNode& b)
{
    return a.distTo(b);
}

double LatNode::distToMS(LatNode& f)
{
    double res = latsMS[n][f.n] + (*distr2)(*generator);
    cout << "LatNodes: " << n << " to " << f.n << " is " << res << endl;
    return res;
}

uint64_t LatNode::distTo(LatNode& f)
{
    uint64_t res = lats[n][f.n] + 1000000*((*distr2)(*generator));
    cout << "LatNodes: " << n << " to " << f.n << " is " << res << " ns." << endl;
    return res;
}
