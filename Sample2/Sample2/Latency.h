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
    double distToMS(LatNode& f);
    uint64_t distTo(LatNode& f);
    
    friend double distMS(LatNode& a, LatNode& b);
    friend uint64_t dist(LatNode& a, LatNode& b);

private:
    friend class LatNodeStatCons;
    struct LatNodeStatCons
    {
        LatNodeStatCons()
        {
            int seed = std::chrono::system_clock::now().time_since_epoch().count();
            LatNode::generator = new std::default_random_engine(seed);
            LatNode::distr = new std::uniform_int_distribution<int>(0, 10);
            LatNode::distr2 = new std::uniform_int_distribution<int>(0, 40);
        }
    };
    static LatNodeStatCons stat_cons;
    static std::default_random_engine* generator;
    static std::uniform_int_distribution<int>* distr;
    static std::uniform_int_distribution<int>* distr2;

    const double latsMS[11][11] = 
    {
        { 0,          297.55,    18.138,   117.09,     9.221,     135.735,   44.079,    78.237,    20.559,    21.099,    242.012 },
        { 297.598,    0,         284.805,  195.108,    266.656,   179.024,   349.805,   251.246,   290.88,    286.576,   186.217 },
        { 18.194,     284.76,    0,        131.298,    20.813,    151.801,   43.295,    93.801,    22.369,    9.664,     264.723 },
        { 114.208,    191.232,   127.108,  0,          105.519,   37.966,    168.777,   38.207,    109.244,   137.878,   135.681 },
        { 9.34,       266.602,   20.999,   110.453,    0,         130.906,   54.245,    70.909,    4.024,     26.017,    226.971 },
        { 135.649,    178.908,   151.868,  38.537,     130.894,   0,         182.548,   68.834,    132.812,   151.923,   107.806 },
        { 44.043,     349.92,    43.802,   174.814,    54.157,    182.44,    0,         126.169,   53.805,    21.123,    206.7 },
        { 78.456,     251.235,   93.857,   37.19,      70.905,    68.833,    126.053,   0,         80.151,    91.753,    209.745 },
        { 22.614,     291.133,   22.898,   117.024,    3.899,     134.253,   52.562,    72.459,    0,         24.261,    226.361 },
        { 21.11,      286.469,   9.732,    142.554,    26.116,    151.97,    21.001,    101.839,   26.284,    0,         277.991 },
        { 242.055,    185.781,   264.59,   137.335,    226.981,   107.602,   206.576,   209.836,   222.891,   278.545,   0 }
    };

    const uint64_t lats[11][11] =
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
double distMS(LatNode& a, LatNode& b);
uint64_t dist(LatNode& a, LatNode& b);
