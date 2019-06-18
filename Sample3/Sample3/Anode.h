#pragma once

//! Simulates A node
SC_MODULE(Anode) {
    int number;
    // currently N/A
    std::unordered_map<int, int> a_memory;

    WSAP cc;
    inMSGNP msgn_snd;

    int kblk_rec;
    int lbblk_rec;
    int msgnblk_snt;

    // S forwards k_blocks to A
    // we do not care about single A node network bandwidth
    inK k_rec;
    inLB lb_rec;

    //double taprocessk;
    //double taprocesslb;

    int send_msgn;
    msgn_block msgn_tosend;

    SC_HAS_PROCESS(Anode);
    Anode(sc_module_name name, int num_, WSAP cc_, //WnodeP Wconn_, SnodeP Sconn_
          log_fifo<msgn_block>* outport) : sc_module(name),
        k_rec("A_k_rec_fifo", 10), kblk_rec(0),
        lb_rec("A_lb_rec_fifo", 10), lbblk_rec(0),
        msgn_snd(outport), msgnblk_snt(0),
        send_msgn(false),
        cc(cc_),
        number(num_)
    {
        //taprocessk = confDbl(cnf_taprocessk);
        //taprocesslb = confDbl(cnf_taprocesslb);
        SC_METHOD(CheckIncoming);
        sensitive << k_rec.data_written_event() << lb_rec.data_written_event();
        dont_initialize();
    }

    ~Anode()
    {
        if ((1 != kblk_rec) || (1 != lbblk_rec) || (1 != msgnblk_snt))
        {
            cout << "Destructor,Anode," << name() << "," << kblk_rec << "," << lbblk_rec << "," << msgnblk_snt << endl;
        }
        //cout << "Destructed " << name() << "." << endl;
    }

    //void main();
    void CheckIncoming();
    int SendToConnectedNode(msgn_block& k);
};
typedef Anode* AnodeP;
