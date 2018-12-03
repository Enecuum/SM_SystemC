#pragma once

SC_MODULE(Wnode) {
    int number;
    // currently N/A
    std::unordered_map<int, int> w_memory;

    int kblk_gen;
    int kblk_rec;
    int mblk_rec;

    WSAP cc;

    // W sends (broadcasts) k_blocks to S and _W_
    log_fifo<k_block> k_rec;
    // A reacts on recv_shadow_rq with m_block to S and _W_
    log_fifo<m_block> m_rec;

    SC_HAS_PROCESS(Wnode);
    Wnode(sc_module_name name, int num_, WSAP cc_) : sc_module(name),
        k_rec("W_k_rec_fifo", 10), m_rec("W_m_rec_fifo", 10),
        kblk_gen(0), kblk_rec(0), mblk_rec(0),
        cc(cc_), number(num_)
    {
        SC_THREAD(main);
    }

    ~Wnode()
    {
        cout << "Destructor,Wnode," << name() << "," << kblk_gen << "," << kblk_rec << "," << mblk_rec << endl;
        //cout << "Destructing " << name() << ": kgen=" << kblk_gen << "   krec=" << kblk_rec << "   mrec=" << mblk_rec << endl;
        //cout << "Destructed " << name() << "." << endl;
    }

    void main();

    k_block GenerateKBlock();
    void Broadcast(k_block k);
    void CheckIncoming();
};
typedef Wnode* WnodeP;
