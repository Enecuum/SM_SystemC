#include "stdafx.h"

std::default_random_engine* LatNode::generator;
std::uniform_int_distribution<int>* LatNode::distr;
std::uniform_int_distribution<int>* LatNode::distr2;
LatNode::LatNodeStatCons LatNode::stat_cons;

LatNode::LatNode()
{
    n = (*distr)(*generator);
}


double dist(LatNode& a, LatNode& b)
{
    return a.distTo(b);
}

double LatNode::distTo(LatNode& f)
{
    double res = lats[n][f.n] + (*distr2)(*generator);
    //cout << "LatNodes," << n << "," << f.n << "," << res << " ns" << endl;
    return res;
}

double randomALatency()
{
    double res = (*LatNode::distr2)(*LatNode::generator);
    //cout << "randomALatency," << res << " ns" << endl;
    return res;
}

void redefineRandomLatency(int from, int to)
{
    if (LatNode::distr2)
    {
        delete LatNode::distr2;
    }
    LatNode::distr2 = new std::uniform_int_distribution<int>(from, to);
}
