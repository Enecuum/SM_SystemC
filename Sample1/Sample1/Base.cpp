#include "stdafx.h"

double baseMu = 10000;
double baseSigma = 10000;
int baseWnum = 2;
int baseSnum = 2;
int baseAnum = 10;
int t_proc = 100;
int t_send = 100;
int t_broadcast = 100;

// positive only
// probably it was supposed to be exponential distribution? easy to change anyway...
int randomTime()
{
    return randomTime(baseMu, baseSigma);
}

int randomTime(double mu, double sigma)
{
    static std::default_random_engine generator;
    std::normal_distribution<double> distribution(mu, sigma);
    int number = (int)abs(distribution(generator));
    //std::cout << "randomTime() = " << number << " (mu,sigma) = (" << baseMu << "," << baseSigma << ") " << endl;
    return number;
}

int randomTime(WnodeP w)
{
    return randomTime(baseMu, baseSigma);
}

int randomTime(WnodeP w, m_block& m)
{
    return t_proc;
}

int randomTime(WnodeP w, k_block& k)
{
    return t_proc;
}

int randomTime(WSAP p, k_block& k)
{
    return t_broadcast;
}

int randomTime(SnodeP s, k_block& k)
{
    return t_proc;
}

int randomTime(SnodeP s, send_shadow_rq& k)
{
    return t_proc;
}

int randomTime(SnodeP s, m_block& m)
{
    return t_proc;
}

int randomTime(SnodeP s, recv_shadow_rq& rr)
{
    return t_proc;
}

int randomTime(AnodeP a, k_block& k)
{
    return t_proc;
}

int randomTime(AnodeP a, m_block& k)
{
    return t_proc;
}

int randomTime(AnodeP a, recv_shadow_rq& k)
{
    return t_proc;
}

int randomTime(AnodeP a, send_shadow_rq& k)
{
    return t_proc;
}
