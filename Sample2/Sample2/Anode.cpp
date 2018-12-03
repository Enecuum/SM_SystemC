#include "stdafx.h"

//void Anode::main()
//{
    //while (true)
    //{
    //    CheckIncoming();
    //}
//}

void Anode::CheckIncoming()
{
    k_block k;
    if (k_rec.nb_read(k))
    {
        cout << "Receive," << sc_time_stamp() << "," << name() << "," << k << endl;
        //cout << sc_time_stamp() << ": A " << name() << " received " << k << "." << endl;
        ++kblk_rec;
        // TODO: it kind of sends everything instantly and then waits
        int leader = rand() % 2;
        send_shadow_rq srq = create_send_shadow_rq(leader, k.info1, k.info2, number, NodeType::A);
        int delay = randomTime(this, k);
        delay += SendToConnectedNode(srq);
        next_trigger(delay, SC_NS);
        return; // !!!
    }
    else
    {
        recv_shadow_rq rr;
        if (recv_rec.nb_read(rr))
        {
            cout << "Receive," << sc_time_stamp() << "," << name() << "," << rr << endl;
            //cout << sc_time_stamp() << ": A " << name() << " received " << rr << "." << endl;
            ++rblk_rec;
            m_block m = create_m_block(rr.info1, rr.info2, NodeType::A);
            int delay = randomTime(this, rr);
            delay += SendToConnectedNode(m);
            next_trigger(delay, SC_NS);
            return; // !!!
        }
    }
    next_trigger();
}

int Anode::SendToConnectedNode(send_shadow_rq& srq)
{
    int delay = 0;
    if (NULL != Sconn)
    {
        Sconn->send_rec.nb_write(srq);
        ++sblk_snt;
        delay += randomTime(this, srq);
    }
    return delay;
}

int Anode::SendToConnectedNode(m_block& m)
{
    int delay = 0;
    if (NULL != Wconn)
    {
        Wconn->m_rec.nb_write(m);
        ++mblk_snt;
        delay += randomTime(this, m);
    }
    if (NULL != Sconn)
    {
        Sconn->m_rec.nb_write(m);
        ++mblk_snt;
        delay += randomTime(this, m);
    }
    return delay;
}
