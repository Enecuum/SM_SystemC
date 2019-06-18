#pragma once

#include "stdafx.h"

// Some latency simulation with some sample pings from https://wondernetwork.com/pings
//             Amsterdam    Auckland    Copenhagen  Dallas      London      Los Angeles Moscow      New York    Paris       Stockholm   Tokyo
// Amsterdam   0,           297.55ms    18.138ms   117.09ms     9.221ms     135.735ms   44.079ms    78.237ms    20.559ms    21.099ms    242.012ms
// Auckland    297.598ms    —           284.805ms  195.108ms    266.656ms   179.024ms   349.805ms   251.246ms   290.88ms    286.576ms   186.217ms
// Copenhagen  18.194ms     284.76ms    —          131.298ms    20.813ms    151.801ms   43.295ms    93.801ms    22.369ms    9.664ms     264.723ms
// Dallas      114.208ms    191.232ms   127.108ms  —            105.519ms   37.966ms    168.777ms   38.207ms    109.244ms   137.878ms   135.681ms
// London      9.34ms       266.602ms   20.999ms   110.453ms    —           130.906ms   54.245ms    70.909ms    4.024ms     26.017ms    226.971ms
// Los Angeles 135.649ms    178.908ms   151.868ms  38.537ms     130.894ms   —           182.548ms   68.834ms    132.812ms   151.923ms   107.806ms
// Moscow      44.043ms     349.92ms    43.802ms   174.814ms    54.157ms    182.44ms    —           126.169ms   53.805ms    21.123ms    206.7ms
// New York    78.456ms     251.235ms   93.857ms   37.19ms      70.905ms    68.833ms    126.053ms    —          80.151ms    91.753ms    209.745ms
// Paris       22.614ms     291.133ms   22.898ms   117.024ms    3.899ms     134.253ms   52.562ms    72.459ms    —           24.261ms    226.361ms
// Stockholm   21.11ms      286.469ms   9.732ms    142.554ms    26.116ms    151.97ms    21.001ms    101.839ms   26.284ms    —           277.991ms
// Tokyo       242.055ms    185.781ms   264.59ms   137.335ms    226.981ms   107.602ms   206.576ms   209.836ms   222.891ms   278.545ms   —
//
// In the same city? Let the latency be 0.
// But add random number 0-40 ms. Because a couple of tests show 17-35 ms inside SPb.
// In different cities? Add the same random number.
// What is the distribution of this extra time? Who knows... Gaussian?
//
// https://pdos.csail.mit.edu/archive/p2psim/kingdata/ --- some "real life" latency data, but from 2005.
// There are many approaches to latency simulation. See "topologies" in p2psim for examples.

struct LatNode
{
    int n;

    LatNode();
    double distTo(LatNode& f);
    
    friend double dist(LatNode& a, LatNode& b);

    friend double randomALatency();

    friend void redefineRandomLatency(int from, int to, int nodeCount, double votingTime);

    double nextVoteLat();
    int    nextVote();

private:
    friend class LatNodeStatCons;
    struct LatNodeStatCons
    {
        LatNodeStatCons()
        {
            uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
            LatNode::generator = new std::default_random_engine((unsigned int)seed);
            LatNode::distr = new std::uniform_int_distribution<int>(0, 10); // hardcoded as we have world latencies hardcoded
            LatNode::distr2 = new std::uniform_int_distribution<int>(0, 40); // using config in this "static constructor" is earlier than parsing config file, so we'll overwrite this variable after parsing config file
            //LatNode::distr2 = new std::normal_distribution<int>(20, 5);
            LatNode::distrVote = new std::uniform_int_distribution<int>(0, 40); // will be overwritten
            votingTime = 10000; // will be overwritten
        }
    };
    static LatNodeStatCons stat_cons;
    static std::default_random_engine* generator;
    static std::uniform_int_distribution<int>* distr;
    static std::uniform_int_distribution<int>* distr2;
    //static std::normal_distribution<int>* distr2;
    static std::uniform_int_distribution<int>* distrVote;

    static double votingTime;

    const double lats[11][11] =
    {
        { 0,            297550000,   18138000,   117090000,    9221000,     135735000,   44079000,    78237000,    20559000,    21099000,    242012000 },
        { 297598000,    0,           284805000,  195108000,    266656000,   179024000,   349805000,   251246000,   290880000,   286576000,   186217000 },
        { 18194000,     284760000,   0,          131298000,    20813000,    151801000,   43295000,    93801000,    22369000,    9664000,     264723000 },
        { 114208000,    191232000,   127108000,  0,            105519000,   37966000,    168777000,   38207000,    109244000,   137878000,   135681000 },
        { 9340000,      266602000,   20999000,   110453000,    0,           130906000,   54245000,    70909000,    4024000,     26017000,    226971000 },
        { 135649000,    178908000,   151868000,  38537000,     130894000,   0,           182548000,   68834000,    132812000,   151923000,   107806000 },
        { 44043000,     349920000,   43802000,   174814000,    54157000,    182440000,   0,           126169000,   53805000,    21123000,    206700000 },
        { 78456000,     251235000,   93857000,   37190000,     70905000,    68833000,    126053000,   0,           80151000,    91753000,    209745000 },
        { 22614000,     291133000,   22898000,   117024000,    3899000,     134253000,   52562000,    72459000,    0,           24261000,    226361000 },
        { 21110000,     286469000,   9732000,    142554000,    26116000,    151970000,   21001000,    101839000,   26284000,    0,           277991000 },
        { 242055000,    185781000,   264590000,  137335000,    226981000,   107602000,   206576000,   209836000,   222891000,   278545000,   0 }
    };
};
double dist(LatNode& a, LatNode& b);
double randomALatency();
void redefineRandomLatency(int from, int to, int nodeCount, double votingTime);
