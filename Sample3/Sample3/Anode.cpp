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
    if (send_msgn)
    {
        send_msgn = false;
        ++msgnblk_snt;
        SendToConnectedNode(msgn_tosend);
    }
    else
    {
        k_block k;
        if (k_rec.nb_read(k))
        {
            cout << "Receive," << sc_time_stamp() << "," << name() << "," << k << endl;
            ++kblk_rec;
            next_trigger(taprocessk, SC_NS); // just wait, no need to send anything
            return; // !!!
        }
        else
        {
            lb_block lb;
            if (lb_rec.nb_read(lb))
            {
                cout << "Receive," << sc_time_stamp() << "," << name() << "," << lb << endl;
                ++lbblk_rec;
                //
                send_msgn = true;                 // wait and
                msgn_tosend = create_msgn_block(number, 0, NodeType::A);
                next_trigger(taprocesslb + bwMSGNdelay + randomALatency(), SC_NS); //  send msgn_block
                return; // !!!
            }
        }
    }
    next_trigger();
}

int Anode::SendToConnectedNode(msgn_block& m)
{
    cout << "Sending," << sc_time_stamp() << "," << name() << "," << msgn_snd->name() << "," << m << endl;
    msgn_snd->nb_write(m);
    return 0;
}
