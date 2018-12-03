#pragma once

//! Simulates all A nodes
SC_MODULE(Anode) {
    int number;
    // currently N/A
    std::unordered_map<int, int> a_memory;

    WSAP cc;
    WnodeP Wconn;
    SnodeP Sconn;

    int kblk_rec;
    int rblk_rec;
    int sblk_snt;
    int mblk_snt;
    
    // S forwards k_blocks to A
    log_fifo<k_block> k_rec;
    // S replies to A on send_shadow_rq with recv_shadow_rq
    log_fifo<recv_shadow_rq> recv_rec;

    SC_HAS_PROCESS(Anode);
    Anode(sc_module_name name, int num_, WSAP cc_, WnodeP Wconn_, SnodeP Sconn_) : sc_module(name),
        k_rec("A_k_rec_fifo", 10), recv_rec("A_recv_rec_fifo", 10),
        kblk_rec(0), rblk_rec(0), sblk_snt(0), mblk_snt(0),
        cc(cc_), Wconn(Wconn_), Sconn(Sconn_), number(num_)
    {
        SC_METHOD(CheckIncoming);
        sensitive << k_rec.data_written_event() << recv_rec.data_written_event();
    }

    ~Anode()
    {
        cout << "Destructor,Anode," << name() << "," << kblk_rec << "," << rblk_rec << "," << sblk_snt << "," << mblk_snt << endl;
        //cout << "Destructing " << name() << ": krec=" << kblk_rec << "   rrec=" << rblk_rec << "   ssnt=" << sblk_snt << "   msnt=" << mblk_snt << "." << endl;
        //cout << "Destructed " << name() << "." << endl;
    }

    //void main();
    void CheckIncoming();
    int SendToConnectedNode(send_shadow_rq& srq);
    int SendToConnectedNode(m_block& k);
};
typedef Anode* AnodeP;
