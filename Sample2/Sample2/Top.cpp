#include "stdafx.h"

WSA::WSA(sc_module_name name, int Snum_, int Anum_, int Wno_, int S1no_, int Sleaderno_) : sc_module(name), Snum(Snum_), Anum(Anum_), Wno(Wno_), S1no(S1no_), Sleaderno(Sleaderno_)
{
    std::stringstream namegen;
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
        inMSGNP outport = NULL;
        std::vector<AnodeP>* pconnA = NULL;
        int nodeToConnectTo = rand() % Snum;
        //cout << "Connection,S_" << nodeToConnectTo << "," << "A_" << i << endl;
        pconnA = &(Snodes[nodeToConnectTo]->connAs);
        outport = &(Snodes[nodeToConnectTo]->msgn_rec);
        namegen << "_" << nodeToConnectTo;
        AnodeP newA = new Anode(namegen.str().c_str(), i, this, outport);
        Anodes[i] = newA;
        pconnA->push_back(newA);
    }

    SC_THREAD(main);
}

WSA::~WSA()
{
    cout << "Destructor,top,start" << endl;
    //cout << "Destructing top." << endl;
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

void WSA::BroadcastK(SnodeP w, k_block k)
{
    for (int i = 0; i < Snum; ++i)
    {
        Snodes[i]->k_rec.nb_write(k);
        //wait(randomTime(this, k), SC_NS);
    }
    wait(1000 /*randomTime(this, k)*/, SC_NS);
}
