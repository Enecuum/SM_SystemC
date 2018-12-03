#include "stdafx.h"

//void Snode::main()
//{
    //while (true)
    //{
    //    CheckIncoming();
    //}
//}

void Snode::CheckIncoming()
{
    k_block k;
    if (k_rec.nb_read(k))
    {
        cout << "Receive," << sc_time_stamp() << "," << name() << "," << k << endl;
        //cout << sc_time_stamp() << ": S " << name() << " received " << k << "." << endl;
        ++kblk_rec;
        k.sender = NodeType::S;
        // TODO: it kind of sends everything instantly and then waits
        int delay = randomTime(this, k);
        delay += SendToConnectedA(k);
        next_trigger(delay, SC_NS);
        return; // !!!
    }
    else
    {
        send_shadow_rq srq;
        if (send_rec.nb_read(srq))
        {
            cout << "Receive," << sc_time_stamp() << "," << name() << "," << srq << endl;
            //cout << sc_time_stamp() << ": S " << name() << " received " << srq << "." << endl;
            ++sblk_rec;
            int delay = randomTime(this, srq);
            if (srq.leader)
            {
                recv_shadow_rq rr = create_recv_shadow_rq(srq.info1, srq.info2, NodeType::S);
                delay += randomTime(this, rr);
                delay += ReplyToA(srq.info3, rr);
                next_trigger(delay, SC_NS);
                return; // !!!
            }
            else
            {
                next_trigger(delay, SC_NS);
                return; // !!!
            }
        }
        else
        {
            m_block m;
            if (m_rec.nb_read(m))
            {
                cout << "Receive," << sc_time_stamp() << "," << name() << "," << m << endl;
                //cout << sc_time_stamp() << ": S " << name() << " received " << m << "." << endl;
                ++mblk_rec;
                next_trigger(randomTime(this, m), SC_NS);
                return; // !!!
            }
        }
    }
    next_trigger();
}

int Snode::SendToConnectedA(k_block& k)
{
    int delay = 0;
    for (int i = 0; i < connAs.size(); ++i)
    {
        connAs[i]->k_rec.nb_write(k);
        ++kblk_snt;
        delay += randomTime(this, k);
    }
    return delay;
}

int Snode::ReplyToA(int Anumber, recv_shadow_rq& rr)
{
    int delay = 0;
    cc->Anodes[Anumber]->recv_rec.nb_write(rr);
    ++rblk_snt;
    delay += randomTime(this, rr);
    return delay;
}
