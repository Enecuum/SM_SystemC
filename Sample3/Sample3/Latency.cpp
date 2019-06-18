#include "stdafx.h"

std::default_random_engine* LatNode::generator;
std::uniform_int_distribution<int>* LatNode::distr;
std::uniform_int_distribution<int>* LatNode::distr2;
std::uniform_int_distribution<int>* LatNode::distrVote;
double LatNode::votingTime;
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

double LatNode::nextVoteLat()
{
    double res = votingTime;
    return res;
}

int LatNode::nextVote()
{
    int res = (*LatNode::distrVote)(*LatNode::generator);
    return res;
}

double randomALatency()
{
    double res = (*LatNode::distr2)(*LatNode::generator);
    //cout << "randomALatency," << res << " ns" << endl;
    return res;
}

void redefineRandomLatency(int from, int to, int nodeCount, double votingTime)
{
    if (LatNode::distr2)
    {
        delete LatNode::distr2;
        delete LatNode::distrVote;
    }
    LatNode::distr2 = new std::uniform_int_distribution<int>(from, to);
    LatNode::distrVote = new std::uniform_int_distribution<int>(0, nodeCount - 1);
    LatNode::votingTime = votingTime;
}
