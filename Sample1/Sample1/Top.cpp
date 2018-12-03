#include "stdafx.h"

WSA::WSA(sc_module_name name, int Wnum_, int Snum_, int Anum_) : sc_module(name), Wnum(Wnum_), Snum(Snum_), Anum(Anum_)
{
    std::stringstream namegen;
    Wnodes.resize(Wnum);
    for (int i = 0; i < Wnum; ++i)
    {
        namegen.str(std::string());
        namegen.clear();
        namegen << "W_" << i;
        Wnodes[i] = new Wnode(namegen.str().c_str(), i, this);
    }
    Snodes.resize(Snum);
    for (int i = 0; i < Snum; ++i)
    {
        namegen.str(std::string());
        namegen.clear();
        namegen << "S_" << i;
        Snodes[i] = new Snode(namegen.str().c_str(), i, this);
    }
    Anodes.resize(Anum);
    for (int i = 0; i < Anum; ++i)
    {
        namegen.str(std::string());
        namegen.clear();
        namegen << "A_" << i;
        WnodeP wconn = NULL;
        SnodeP sconn = NULL;
        int nodeToConnectTo = rand() % (Wnum + Snum);
        if (nodeToConnectTo < Wnum)
        {
            cout << "Connection,W_" << nodeToConnectTo << "," << "A_" << i << endl;
            //cout << "W_" << nodeToConnectTo << " <- " << "A_" << i << endl;
            wconn = Wnodes[nodeToConnectTo];
        }
        else
        {
            cout << "Connection,S_" << nodeToConnectTo - Wnum << "," << "A_" << i << endl;
            //cout << "S_" << nodeToConnectTo - Wnum << " <- " << "A_" << i << endl;
            sconn = Snodes[nodeToConnectTo - Wnum];
        }
        Anodes[i] = new Anode(namegen.str().c_str(), i, this, wconn, sconn);
        if (NULL != sconn)
        {
            sconn->connAs.push_back(Anodes[i]);
        }
    }

    SC_THREAD(main);
}

WSA::~WSA()
{
    cout << "Destructor,top,start" << endl;
    //cout << "Destructing top." << endl;
    for (int i = 0; i < Wnum; ++i)
    {
        delete Wnodes[i];
    }
    for (int i = 0; i < Snum; ++i)
    {
        delete Snodes[i];
    }
    for (int i = 0; i < Anum; ++i)
    {
        delete Anodes[i];
    }
    cout << "Destructor,top,end" << endl;
    //cout << "Destructed top." << endl;
}

void WSA::main()
{
    cout << "NodeStart," << sc_time_stamp() << "," << name() << endl;
    //cout << sc_time_stamp() << ": WSA " << name() << " main is running." << endl;
}

void WSA::BroadcastK(WnodeP w, k_block k)
{
    for (int i = 0; i < Wnum; ++i)
    {
        Wnodes[i]->k_rec.nb_write(k);
        //wait(randomTime(this, k), SC_NS);
    }
    for (int i = 0; i < Snum; ++i)
    {
        Snodes[i]->k_rec.nb_write(k);
        //wait(randomTime(this, k), SC_NS);
    }
    wait(randomTime(this, k), SC_NS);
}
