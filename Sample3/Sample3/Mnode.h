#pragma once

SC_MODULE(Mnode) {
    int number;

    // node kind
    int is_S1;
    int is_Sleader;
    int is_W;

    // state flags
    int sent_votes;

    int sent_k;
    int sent_shrq;
    int sent_shpn;
    int sent_lb;
    int broad_k;
    int broad_lb;
    int got_k;
    int got_shrq;
    int got_shpn;
    int got_lb;
    int got_m;
    int total_msigns;

    std::vector<int> fingers;
    int fingerM;

    WSAP cc;
    std::vector<AnodeP> connAs;

    int kblk_rec;
    int kblk_snt;
    int srqblk_rec;
    int srqblk_snt;
    int spnblk_rec;
    int spnblk_snt;
    int lbblk_rec;
    int lbblk_snt;
    int msgnblk_rec;
    int msgnblk_snt;
    int mblk_rec;
    int mblk_snt;
    int routed_rec;
    int routed_snt;

    // S routes k, srq, spn, lb, msgn, m blocks
    inK k_rec;
    inSRQ srq_rec;
    inSPN spn_rec;
    inLB lb_rec;
    inMSGN msgn_rec;
    inM m_rec;
    inR r_rec;

    // managing network latency
    LatNode latNode;
    sc_event_queue eqSend;
    sc_event_queue eqVote;
    std::multimap<uint64, std::pair<k_block, inKP>> delayed_k;
    std::multimap<uint64, std::pair<lb_block, inLBP>> delayed_lb;
    std::multimap<uint64, std::pair<routing_block, inRP>> delayed_r;

    // managing disk speed
    uint64 lastUpdateTime;
    const uint64 updateAfter = 100000000;
    uint64 lastLogTime;
    double currentDiskBuffer;


    SC_HAS_PROCESS(Mnode);
    Mnode(sc_module_name name, int num_, WSAP cc_) : sc_module(name),
        k_rec("S_k_rec_fifo", 10), srq_rec("S_srq_fifo", 10), spn_rec("S_spn_fifo", 10), lb_rec("S_lb_fifo", 10), msgn_rec("S_msgn_fifo", bigFifo), m_rec("S_m_fifo", 10), r_rec("S_r_fifo", bigFifo),
        kblk_rec(0), kblk_snt(0), srqblk_rec(0), srqblk_snt(0), spnblk_rec(0), spnblk_snt(0), lbblk_rec(0), lbblk_snt(0), msgnblk_rec(0), msgnblk_snt(0), mblk_rec(0), mblk_snt(0), routed_rec(0), routed_snt(0),
        cc(cc_), number(num_),
        sent_k(0), sent_shrq(0), sent_shpn(0), sent_lb(0), sent_votes(0),
        broad_k(0), broad_lb(0),
        got_k(0), got_shrq(0), got_shpn(0), got_lb(0), got_m(0),
        total_msigns(0),
        lastUpdateTime(0), lastLogTime(0), currentDiskBuffer(0)
    {
        is_W = (cc->Wno == number);
        is_S1 = (cc->S1no == number);
        is_Sleader = (cc->Sleaderno == number);

        SC_THREAD(main);
        //dont_initialize();
        SC_METHOD(delayed_send);
        sensitive << eqSend;
        dont_initialize();

        InitializeFingers();
    }

    ~Mnode()
    {
        //cout << "Destructor,Mnode," << name() << "," --- verbose vote casting information?
        int error = false;
        if (0 < delayed_k.size())
        {
            error = true;
            cout << "InternalError,Mnode," << name() << ",delayed_k is not empty: " << delayed_k.size() << endl;
        }
        if (0 < delayed_lb.size())
        {
            error = true;
            cout << "InternalError,Mnode," << name() << ",delayed_lb is not empty: " << delayed_lb.size() << endl;
        }
        //if (!got_m)
        //{
        //    error = true;
        //    cout << "Error,Mnode," << name() << ",did not receive m_block" << endl;
        //}
        //if (is_Sleader && (total_msigns < cc->Anum))
        //{
        //    error = true;
        //    cout << "Error,Mnode," << name() << ",did not receive all msigns" << endl;
        //}
        if (error)
        {
            cout << "Destructor,Mnode," << name() << ",k," << kblk_rec << "," << kblk_snt <<
                ",q," << srqblk_rec << "," << srqblk_snt <<
                ",p," << spnblk_rec << "," << spnblk_snt <<
                ",l," << lbblk_rec << "," << lbblk_snt <<
                ",g," << msgnblk_rec << "," << msgnblk_snt <<
                ",m," << mblk_rec << "," << mblk_snt <<
                ",r," << routed_rec << "," << routed_snt <<
                connAs.size() << endl;
        }
        //if (is_Sleader)
        //{
        //    sc_time t = sc_time::from_value(lastUpdateTime);
        //    double toFinish = 0;
        //    if (currentDiskBuffer > 0)
        //    {
        //        toFinish = currentDiskBuffer / diskspeed;
        //    }
        //    t += sc_time(toFinish, SC_NS);
        //    cout << "Destructor,Leader," << name() << ",unwrittenBuffer = " << currentDiskBuffer << "   timeToFinish = " << toFinish << "   willFinishAt = " << t << endl;
        //}
        //cout << "Destructed " << name() << "." << endl;
    }

    void main();
    void delayed_send();

    int CheckR();
    void SendToConnectedA(k_block& k);
    void SendToConnectedA(lb_block& lb);
    void InitializeFingers();
    //! receive block and/or send it further
    void ProcessRoutingBlock(routing_block& rb, double xdelay = 0);
    //! just process block according to its contents (routing part of routing block is ignored)
    void ProcessBlock(routing_block& rb, double xdelay = 0);
    //! just send routing block (not processing its routing information) to finger f
    void SendBlock(routing_block& rb, int nodenum, double xdelay = 0);
    //! broadcasting routine
    void ForwardBlock(routing_block& rb, double xdelay = 0);
};
typedef Mnode* MnodeP;
