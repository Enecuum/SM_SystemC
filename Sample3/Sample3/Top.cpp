#include "stdafx.h"

WSA::WSA(sc_module_name name, int Snum_, int Anum_, int Wno_, int S1no_, int Sleaderno_, int Mno_, int Rno_, int rqV_, int dV_, int sAW_)
    : sc_module(name), Snum(Snum_), Anum(Anum_), Wno(Wno_), S1no(S1no_), Sleaderno(Sleaderno_), Mnum(Mno_), rndNum(Rno_),
    rqVotes(rqV_), currMaxVotes(0), dumpVotes(dV_), currWinners(0), stopWinners(sAW_)
{
    std::stringstream namegen;
    //Snodes.resize(Snum);
    //for (int i = 0; i < Snum; ++i)
    //{
    //    namegen.str(std::string());
    //    namegen.clear();
    //    namegen << "S_" << i;
    //    Snodes[i] = new Snode(namegen.str().c_str(), i, this);
    //}
    //Anodes.resize(Anum);
    //for (int i = 0; i < Anum; ++i)
    //{
    //    namegen.str(std::string());
    //    namegen.clear();
    //    namegen << "A_" << i;
    //    inMSGNP outport = NULL;
    //    std::vector<AnodeP>* pconnA = NULL;
    //    int nodeToConnectTo = rand() % Snum;
    //    //cout << "Connection,S_" << nodeToConnectTo << "," << "A_" << i << endl;
    //    pconnA = &(Snodes[nodeToConnectTo]->connAs);
    //    outport = &(Snodes[nodeToConnectTo]->msgn_rec);
    //    namegen << "_" << nodeToConnectTo;
    //    AnodeP newA = new Anode(namegen.str().c_str(), i, this, outport);
    //    Anodes[i] = newA;
    //    pconnA->push_back(newA);
    //}
    Mnodes.resize(Mnum);
    Votes.resize(Mnum);
    for (int i = 0; i < Mnum; ++i)
    {
        namegen.str(std::string());
        namegen.clear();
        namegen << "M_" << i;
        Mnodes[i] = new Mnode(namegen.str().c_str(), i, this);
        Votes[i].clear(); // not necessary
    }
    winTimes.resize(stopWinners);

    SC_THREAD(main);
}

WSA::~WSA()
{
    //### cout << "Destructor,top,start" << endl;
    //cout << "Destructing top." << endl;
    //for (int i = 0; i < Snum; ++i)
    //{
    //    delete Snodes[i];
    //}
    //for (int i = 0; i < Anum; ++i)
    //{
    //    delete Anodes[i];
    //}
    for (int i = 0; i < Mnum; ++i)
    {
        delete Mnodes[i];
    }
    if (dumpVotes)
    {
        int totalDifferentVotes = 0;
        int totalOverRq = 0;
        for (int i = 0; i < Votes.size(); ++i)
        {
            int thisOneCast = 0;
            for (int k = 0; k < Mnum; ++k)
            {
                if (Votes[k].find(i) != Votes[k].end())
                {
                    ++thisOneCast;
                }
            }
            if (dumpVotes > 2) cout << "Votes for " << i << " (total: " << Votes[i].size() << ", cast: " << thisOneCast << "): ";
            if (Votes[i].size() >= rqVotes)
            {
                ++totalOverRq;
            }
            totalDifferentVotes += Votes[i].size();
            for (int j = 0; j < Mnum; ++j)
            {
                if (Votes[i].find(j) != Votes[i].end())
                {
                    if (dumpVotes > 3) cout << j << " ";
                }
            }
            if (dumpVotes > 3) cout << endl;
        }
        if (dumpVotes > 1) cout << "Total different votes: " << totalDifferentVotes << " (out of " << Mnum*rndNum << ")." << endl;
        if (dumpVotes > 1) cout << "Total winners: " << totalOverRq << endl;
        cout << "WinTimes,";
        if (currWinners == 0)
        {
            cout << "NO WINNERS";
        }
        for (int i = 0; i < winTimes.size() && i < currWinners; ++i)
        {
            cout << winTimes[i] << ",";
        }
        cout << endl;
    }
    //### cout << "Destructor,top,end" << endl;
    //cout << "Destructed top." << endl;
}

void WSA::main()
{
    //cout << "NodeStart," << sc_time_stamp() << "," << name() << endl;
    //cout << sc_time_stamp() << ": WSA " << name() << " main is running." << endl;
}

void WSA::BroadcastK(SnodeP w, k_block k)
{
    for (int i = 0; i < Snum; ++i)
    {
        Snodes[i]->k_rec.nb_write(k);
        //wait(randomTime(this, k), SC_NS);
    }
    wait(1000 /*randomTime(this, k)*/, SC_NS);
}
