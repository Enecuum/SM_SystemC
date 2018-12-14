#include "stdafx.h"

double taprocessk;
double taprocesslb;

double bwKdelay;
double bwShRqdelay;
double bwShPndelay;
double bwLBdelay;
double bwMSGNdelay;
double bwMdelay;

double dbK;
double dbShRq;
double dbShPn;
double dbLB;
double dbMS;
double dbM;

double diskspeed;

void InitBWDB()
{


    double bw = confDbl(cnf_bandwidth);
    double ksize = confInt(cnf_ksize);
    double rqsize = confInt(cnf_ShRqsize);
    double pnsize = confInt(cnf_ShPnsize);
    double lbsize = confInt(cnf_lbsize);
    double msgnsize = confInt(cnf_msgnsize);
    double msize = confInt(cnf_msize);

    diskspeed = confDbl(cnf_diskbw);

    bwKdelay = ksize / bw;
    bwShRqdelay = rqsize / bw;
    bwShPndelay = pnsize / bw;
    bwLBdelay = lbsize / bw;
    bwMSGNdelay = msgnsize / bw;
    bwMdelay = msize / bw;

    dbK = ksize;
    dbShRq = rqsize;
    dbShPn = pnsize;
    dbLB = lbsize;
    dbMS = msgnsize;
    dbM = msize;
}

double bwDelay(BlockType::BlockType bt)
{
    if (BlockType::K == bt) return bwKdelay;
    if (BlockType::ShRq == bt) return bwShRqdelay;
    if (BlockType::ShPn == bt) return bwShPndelay;
    if (BlockType::LB == bt) return bwLBdelay;
    if (BlockType::MSgn == bt) return bwMSGNdelay;
    if (BlockType::M == bt) return bwMdelay;
    assert(0);
    return 0;
}
