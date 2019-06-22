#pragma once

SC_MODULE(WSA)
{
    std::vector<SnodeP> Snodes;
    std::vector<AnodeP> Anodes;
    std::vector<MnodeP> Mnodes;

    int dumpVotes;
    std::vector<std::set<int>> Votes;

    int Snum;
    int Anum;
    int Mnum;

    int rndNum;
    int currMaxVotes;
    int rqVotes;
    int currWinners;
    int stopWinners;
    std::vector<uint64> winTimes;

    int Wno;
    int S1no;
    int Sleaderno;

    void BroadcastK(SnodeP w, k_block k);

    SC_HAS_PROCESS(WSA);
    WSA(sc_module_name name, int Snum_, int Anum_, int Wno_, int S1no_, int Sleaderno_, int Mno_, int Rno_, int rqV_, int dV_, int sAW_);
    ~WSA();

    void main();
};
typedef WSA* WSAP;
