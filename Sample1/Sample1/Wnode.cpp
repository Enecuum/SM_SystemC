#include "stdafx.h"

void Wnode::main()
{
    cout << "NodeStart," << sc_time_stamp() << "," << name() << endl;
    //cout << sc_time_stamp() << ": W " << name() << " main is running." << endl;
    while (true)
    {
        k_block k = GenerateKBlock();
        cout << "Generated," << sc_time_stamp() << "," << name() << "," << k << endl;
        //cout << sc_time_stamp() << ": W " << name() << " generated " << k << "." << endl;
        Broadcast(k);
        CheckIncoming();
    }
}

k_block Wnode::GenerateKBlock()
{
    wait(randomTime(this), SC_NS);
    k_block k = create_k_block(0, number, kblk_gen, NodeType::W);
    ++kblk_gen;
    return k;
}

void Wnode::Broadcast(k_block k)
{
    cc->BroadcastK(this, k);
    cout << "Broadcast," << sc_time_stamp() << "," << name() << "," << k << endl;
    //cout << sc_time_stamp() << ": W " << name() << " broadcasted " << k << "." << endl;
}

void Wnode::CheckIncoming()
{
    k_block k;
    while (k_rec.nb_read(k))
    {
        cout << "Receive," << sc_time_stamp() << "," << name() << "," << k << endl;
        //cout << sc_time_stamp() << ": W " << name() << " received " << k << "." << endl;
        ++kblk_rec;
        wait(randomTime(this, k), SC_NS);
    }
    m_block m;
    while (m_rec.nb_read(m))
    {
        cout << "Receive," << sc_time_stamp() << "," << name() << "," << m << endl;
        //cout << sc_time_stamp() << ": W " << name() << " received " << m << "." << endl;
        ++mblk_rec;
        wait(randomTime(this, m), SC_NS);
    }
}
