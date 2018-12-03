#pragma once

SC_MODULE(Snode) {
    int number;
    // currently N/A
    std::unordered_map<int, int> s_memory;

    WSAP cc;
    std::vector<AnodeP> connAs;

    int kblk_rec;
    int sblk_rec;
    int mblk_rec;
    int kblk_snt;
    int rblk_snt;

    // W sends (broadcasts) k_blocks to _S_ and W
    log_fifo<k_block> k_rec;
    // A reacts on k_block with send_shadow_rq to S
    log_fifo<send_shadow_rq> send_rec;
    // A reacts on recv_shadow_rq with m_block to _S_ and W
    log_fifo<m_block> m_rec;

    SC_HAS_PROCESS(Snode);
    Snode(sc_module_name name, int num_, WSAP cc_) : sc_module(name),
        k_rec("S_k_rec_fifo", 10), send_rec("S_send_rec_fifo", 10), m_rec("S_m_rec_fifo", 10),
        kblk_rec(0), sblk_rec(0), mblk_rec(0), kblk_snt(0), rblk_snt(0),
        cc(cc_), number(num_)
    {
        SC_METHOD(CheckIncoming);
        sensitive << k_rec.data_written_event() << send_rec.data_written_event() << m_rec.data_written_event();
    }

    ~Snode()
    {
        cout << "Destructor,Snode," << name() << "," << kblk_rec << "," << sblk_rec << "," << mblk_rec << "," << kblk_snt << "," << rblk_snt << "," << connAs.size() << endl;
        //cout << "Destructing " << name() << ": krec=" << kblk_rec << " srec=" << sblk_rec << " mrec=" << mblk_rec << " ksnt=" << kblk_snt << " rsnt=" << rblk_snt << " c2a=" << connAs.size() << "." << endl;
        //cout << "Destructed " << name() << "." << endl;
    }

    //void main();
    void CheckIncoming();
    int SendToConnectedA(k_block& k);
    int ReplyToA(int Anumber, recv_shadow_rq& rr);
};
typedef Snode* SnodeP;
