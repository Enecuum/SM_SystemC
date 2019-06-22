#include "stdafx.h"

void Mnode::main()
{
    while (true)
    {
        int didAnything = false;
        // general routing
        while (CheckR()) // handles multiple events in the same moment?
        {
            didAnything = true;
        }
        //
        if (!sent_votes)
        {
            sent_votes = true;
            double totalDelay = 0;
            for (int i = 0; i < cc->rndNum; ++i)
            {
                totalDelay += latNode.nextVoteLat();
                int vote = latNode.nextVote();
                ProcessRoutingBlock(create_routing_block(vote, number, -1, BlockType::LB, NodeType::S, 0, 0), totalDelay);
            }
        }
        //
        //if (is_W && !broad_k) // 1-time, W
        //{
        //    didAnything = true;
        //    broad_k = true;
        //    got_k = true;
        //    ProcessRoutingBlock(create_routing_block(number, number, 0, BlockType::K, NodeType::W, 0, 0));
        //}
        //if (got_k && !sent_k) // 1-time, all
        //{
        //    didAnything = true;
        //    sent_k = true;
        //    k_block k = create_k_block(number, 0, NodeType::S);
        //    cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",started sending k to A" << endl;
        //    SendToConnectedA(k); // potentially long operation
        //    cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",finished sending k to A" << endl;
        //}
        //if (is_Sleader && got_k && !sent_shrq) // 1-time, Sleader
        //{
        //    didAnything = true;
        //    sent_shrq = true;
        //    //UpdateDiskInfo(dbShRq);
        //    ProcessRoutingBlock(create_routing_block(cc->S1no, number, -1, BlockType::ShRq, NodeType::SL, 0, 0));
        //}
        //if (is_S1 && got_k && got_shrq && !sent_shpn) // 1-time, S1
        //{
        //    didAnything = true;
        //    sent_shpn = true;
        //    ProcessRoutingBlock(create_routing_block(cc->Sleaderno, number, -1, BlockType::ShPn, NodeType::S1, 0, 0));
        //}
        //if (is_Sleader && got_k && got_shpn && !broad_lb) // 1-time, Sleader
        //{
        //    didAnything = true;
        //    broad_lb = true;
        //    got_lb = true;
        //    lb_block lb = create_lb_block(number, 0, NodeType::S);
        //    //UpdateDiskInfo(dbLB);
        //    ProcessRoutingBlock(create_routing_block(number, number, 0, BlockType::LB, lb.sender, lb.info1, lb.info2));
        //}
        //// limited receiving options in this case
        //if (got_lb && !sent_lb) // 1-time, all
        //{
        //    didAnything = true;
        //    sent_lb = true;
        //    lb_block lb = create_lb_block(number, 0, NodeType::S);
        //    cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",started sending lb to A" << endl;
        //    SendToConnectedA(lb); // potentially long operation
        //    cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",finished sending lb to A" << endl;
        //    //wait(msgn_rec.data_written_event() | r_rec.data_written_event());
        //}
        // limited receiving options in this case
        //{
        //    msgn_block msgn;
        //    while (msgn_rec.nb_read(msgn))  // handles multiple events in the same moment?
        //    {
        //        didAnything = true;
        //        wait(bwMSGNdelay, SC_NS);
        //        //UpdateDiskInfo(dbMS);
        //        cout << "Receive," << sc_time_stamp() << "," << name() << "," << msgn << endl;
        //        ++msgnblk_rec;
        //        ProcessRoutingBlock(create_routing_block(cc->Sleaderno, number, -1, BlockType::MSgn, NodeType::S, 0, 0));  // heh, Sleader sends them to itself too...
        //        //wait(msgn_rec.data_written_event() | r_rec.data_written_event());
        //    }
        //}
        // nothing will work automatically, so waiting for event
        if (!didAnything)
        {
            //wait(msgn_rec.data_written_event() | r_rec.data_written_event());
            wait(r_rec.data_written_event());
        }
    }
}

void Mnode::delayed_send()
{
    uint64 t = sc_time_stamp().value();
    //{
    //    auto itk = delayed_k.find(t);
    //    if (itk != delayed_k.end())
    //    {
    //        cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",delayed k block <" << itk->second.first << "> at time " << t << endl;
    //        itk->second.second->nb_write(itk->second.first);
    //        delayed_k.erase(itk);
    //        next_trigger();
    //        return; // !!!
    //    }
    //}
    //{
    //    auto itlb = delayed_lb.find(t);
    //    if (itlb != delayed_lb.end())
    //    {
    //        cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",delayed lb block <" << itlb->second.first << "> at time " << t << endl;
    //        itlb->second.second->nb_write(itlb->second.first);
    //        delayed_lb.erase(itlb);
    //        next_trigger();
    //        return; // !!!
    //    }
    //}
    {
        auto itr = delayed_r.find(t);
        if (itr != delayed_r.end())
        {
            //cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",delayed r block <" << itr->second.first << "> at time " << t << endl;
            itr->second.second->nb_write(itr->second.first);
            delayed_r.erase(itr);
            next_trigger();
            return; // !!!
        }
    }
    cout << "InternalError," << sc_time_stamp() << ",Mnode," << name() << ",no delayed block at time " << t << endl;
}

//void Mnode::SendToConnectedA(k_block& k)
//{
//    for (int i = 0; i < connAs.size(); ++i)
//    {
//        // queuing write based on network latency
//        sc_time lat(randomALatency(), SC_NS);
//        sc_time newt = sc_time_stamp() + lat;
//        uint64 val_newt = newt.value();
//        k.info2 = i;
//        cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",queuing k block <" << k << "> to arrive at time " << newt << " ( = " << val_newt << ")" << endl;
//        delayed_k.insert(std::make_pair(val_newt, std::make_pair(k, &(connAs[i]->k_rec))));
//        eqSend.notify(lat);
//        ++kblk_snt;
//        wait(bwKdelay, SC_NS);
//        // sometimes check routing to avoid disrupting chord connectivity
//        if (i % 20 == 0)
//        {
//            CheckR();
//        }
//    }
//}
//
//void Mnode::SendToConnectedA(lb_block& lb)
//{
//    for (int i = 0; i < connAs.size(); ++i)
//    {
//        // queuing write based on network latency
//        sc_time lat(randomALatency(), SC_NS);
//        sc_time newt = sc_time_stamp() + lat;
//        uint64 val_newt = newt.value();
//        lb.info2 = i;
//        cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",queuing lb block <" << lb << "> to arrive at time " << newt << " ( = " << val_newt << ")" << endl;
//        delayed_lb.insert(std::make_pair(val_newt, std::make_pair(lb, &(connAs[i]->lb_rec))));
//        eqSend.notify(lat);
//        ++lbblk_snt;
//        wait(bwLBdelay, SC_NS);
//        // sometimes check routing to avoid disrupting chord connectivity
//        if (i % 20 == 0)
//        {
//            CheckR();
//        }
//    }
//}

int Mnode::CheckR()
{
    routing_block rb;
    if (r_rec.nb_read(rb))
    {
        ProcessRoutingBlock(rb);
        return true; // !!!
    }
    else
    {
        return false; // !!!
    }
}

void Mnode::ProcessBlock(routing_block& rb, double xdelay)
{
    if (BlockType::LB == rb.bt)
    {
        if (cc->Votes[number].find(rb.R) == cc->Votes[number].end())
        {
            cc->Votes[number].insert(rb.R);
            int sz = cc->Votes[number].size();
            if (sz == cc->currMaxVotes)
            {
                //### cout << "Tie," << sc_time_stamp() << ",Mnode," << name() << ",got " << cc->Votes[number].size() << " votes (last vote was from " << rb.R << ")" << endl;
            }
            else if (sz > cc->currMaxVotes)
            {
                cc->currMaxVotes = sz;
                //### cout << "MAX," << sc_time_stamp() << ",Mnode," << name() << ",got " << cc->Votes[number].size() << " votes (last vote was from " << rb.R << ")" << endl;
            }
            if (sz == cc->rqVotes)
            {
                //### cout << "Success," << sc_time_stamp() << ",Mnode," << name() << ",got " << cc->Votes[number].size() << " votes (last vote was from " << rb.R << ")" << endl;
                cc->winTimes[cc->currWinners] = sc_time_stamp().value();
                ++(cc->currWinners);
                if (cc->currWinners == cc->stopWinners)
                {
                    sc_stop(); // !!!
                }
            }
        }
        else
        {
            // already sent a vote to that node
        }
    }
    else
    {
        cout << "InternalError," << sc_time_stamp() << ",Mnode," << name() << ",received not LB block" << endl;
    }
    return;
    //
    //
    // // // !!! !!! !!! we only do LB blocks as voting in this sample
    //
    //
    if (BlockType::K == rb.bt)
    {
        got_k = true;
        //UpdateDiskInfo(dbK);
    }
    else if (is_S1 && (BlockType::ShRq == rb.bt))
    {
        got_shrq = true;
        ProcessRoutingBlock(create_routing_block(cc->Sleaderno, number, -1, BlockType::ShPn, NodeType::S1, 0, 0), xdelay);
    }
    else if (is_Sleader && (BlockType::ShPn == rb.bt))
    {
        got_shpn = true;
        //UpdateDiskInfo(dbShPn);
    }
    else if (BlockType::LB == rb.bt)
    {
        got_lb = true;
    }
    else if (is_Sleader && (BlockType::MSgn == rb.bt))
    {
        ++total_msigns;
        if (total_msigns == cc->Anum)
        {
            //UpdateDiskInfo(dbM);
            ProcessRoutingBlock(create_routing_block(number, number, 0, BlockType::M, NodeType::SL, 0, 0), xdelay);
        }
    }
    else if (BlockType::M == rb.bt)
    {
        got_m = true;
    }
}

void Mnode::SendBlock(routing_block& rb, int nodenum, double xdelay)
{
    MnodeP dstN = cc->Mnodes.at(nodenum);
    sc_time lat(latNode.distTo(dstN->latNode) + xdelay, SC_NS);
    sc_time newt = sc_time_stamp() + lat;
    uint64 val_newt = newt.value();
    //cout << "Trace," << sc_time_stamp() << ",Mnode," << name() << ",queuing routing block <" << rb << "> to arrive at time " << newt << " ( = " << val_newt << ")" << endl;
    delayed_r.insert(std::make_pair(val_newt, std::make_pair(rb, &(dstN->r_rec))));
    eqSend.notify(lat);
    ++routed_snt;
    wait(bwDelay(rb.bt), SC_NS);
}

void Mnode::ProcessRoutingBlock(routing_block& rb, double xdelay)
{
    ++routed_rec;
    int n = cc->Snum;
    if (rb.token < 0) // not broadcast
    {
        if (rb.P != number) // just send further
        {
            int j = (rb.P - number + n) % n; // programming languages like negative remainders for some reason
            int f = 0;
            int i = 1;
            while (i <= j) // j != 0
            {
                ++f;
                i <<= 1;
            }
            --f;
            SendBlock(rb, fingers.at(f), xdelay);
        }
        else // just process
        {
            ProcessBlock(rb, xdelay);
        }
    }
    else // broadcast: often send further, always process
    {
        ForwardBlock(rb, xdelay);
        ProcessBlock(rb, xdelay);
    }
}

void Mnode::InitializeFingers()
{
    fingers.clear();
    int n = cc->Snum;
    // simplified fingers selection algorithm
    for (int i = 1; i < n; i *= 2)
    {
        int j = (i + number) % n;
        fingers.push_back(j);
    }
    fingerM = (int)fingers.size();
    //### cout << "Trace,0,Mnode," << name() << ",fingerM = " << fingerM << "; fingers: ";
    for (const auto i : fingers)
        ; //### cout << i << " ";
    //### cout << endl;
}

void Mnode::ForwardBlock(routing_block& rb, double xdelay)
{
    // balanced broadcasting algorithm described by Huang, Zhang
    if (rb.token >= fingerM)
        return; // !!!
    int newToken = rb.token + 1;
    int fd = fingers.at(rb.token);
    // fd in P--->R or P==R
    if (((rb.P < rb.R) && (rb.P < fd) && (fd < rb.R)) ||
        ((rb.P > rb.R) && ((rb.P < fd) || (fd < rb.R))) ||
        (rb.P == rb.R))
    {
        SendBlock(create_routing_block(fd, rb.R, newToken, rb.bt, rb.sender, rb.info1, rb.info2), fd, xdelay);
    }
    if (newToken < fingerM)
    {
        fd = fingers.at(newToken);
        // fd in P--->R or P==R
        if (((rb.P < rb.R) && (rb.P < fd) && (fd < rb.R)) ||
            ((rb.P > rb.R) && ((rb.P < fd) || (fd < rb.R))) ||
            (rb.P == rb.R))
        {
            SendBlock(create_routing_block(fd, rb.R, newToken, rb.bt, rb.sender, rb.info1, rb.info2), fd, xdelay);
        }
    }
}

