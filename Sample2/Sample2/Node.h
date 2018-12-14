#pragma once

extern int log_fifo_logging_level;
#define log_fifo_log_destructor 9
#define log_fifo_log_ok_io 10
#define log_fifo_log_notok_io 8

namespace NodeType
{
    enum NodeType
    {
        W = 0,
        S = 1,
        S1 = 2,
        SL = 3,
        A = 4,
        Unknown = 999
    };
}
char NTTS(NodeType::NodeType nt);

namespace BlockType
{
    enum BlockType
    {
        K = 0,
        ShRq = 1,
        ShPn = 2,
        LB = 3,
        MSgn = 4,
        M = 5,
        Unknown = 999
    };
}
char BTTS(BlockType::BlockType bt);

struct routing_block
{
    int P; // receiving node
    int R; // source node
    int token; // token (0+ for broadcasting, -1 for unicasting)
    // stuff to create a block
    BlockType::BlockType bt;
    NodeType::NodeType sender;
    int info1;
    int info2;
    friend ostream& operator << (ostream& os, const routing_block& k);
};
routing_block create_routing_block(int P_, int R_, int token_, BlockType::BlockType bt_, NodeType::NodeType nt_, int info1_, int info2_);
ostream& operator << (ostream& os, const routing_block& k);

struct k_block
{
    NodeType::NodeType sender;
	int info1;
    int info2;
	friend ostream& operator << (ostream& os, const k_block& k);
};
k_block create_k_block(int inf1 = 0, int inf2 = 0, NodeType::NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const k_block& k);

struct send_shadow_rq
{
    NodeType::NodeType sender;
	int info1;
    int info2;
    friend ostream& operator << (ostream& os, const send_shadow_rq& k);
};
send_shadow_rq create_send_shadow_rq(int inf1 = 0, int inf2 = 0, NodeType::NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const send_shadow_rq& k);

struct recv_shadow_rq
{
    NodeType::NodeType sender;
    int info1;
    int info2;
    friend ostream& operator << (ostream& os, const recv_shadow_rq& k);
};
recv_shadow_rq create_recv_shadow_rq(int inf1 = 0, int inf2 = 0, NodeType::NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const recv_shadow_rq& k);

struct lb_block
{
    NodeType::NodeType sender;
    int info1;
    int info2;
    friend ostream& operator << (ostream& os, const lb_block& k);
};
lb_block create_lb_block(int inf1_ = 0, int inf2_ = 0, NodeType::NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const lb_block& k);

struct msgn_block
{
    NodeType::NodeType sender;
    int info1;
    int info2;
    friend ostream& operator << (ostream& os, const msgn_block& k);
};
msgn_block create_msgn_block(int inf1 = 0, int inf2 = 0, NodeType::NodeType nt = NodeType::NodeType::Unknown);
ostream& operator << (ostream& os, const msgn_block& k);

struct m_block
{
    NodeType::NodeType sender;
    int info1;
    int info2;
    friend ostream& operator << (ostream& os, const m_block& k);
};
m_block create_m_block(int inf1 = 0, int inf2 = 0, NodeType::NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const m_block& k);


template <class T>
struct log_fifo : sc_fifo<T>
{
    typedef sc_fifo<T> Super;

	log_fifo(sc_module_name name, int size_, int num = 0) : sc_fifo(name, size_)
	{

	}

    bool nb_read(T&) override;
    bool nb_write(const T&) override;

    bool d_write(const T&, double delay);

	~log_fifo()
	{
        if (0 < num_available())
        {
            cout << "DesctructorError,log_fifo," << name() << " available: " << num_available() << endl;
            T obj;
            while (nb_read(obj))
            {
                cout << "DestructorErrorLog,log_fifo," << name() << "," << obj << endl;
            }
        }
        else
        {
            if (log_fifo_log_destructor <= log_fifo_logging_level)
            {
                cout << "Destructor,log_fifo," << name() << endl;
            }
        }
        //cout << "Destructed log_fifo '" << name() << "'." << endl;
    }
};

template<class T>
bool log_fifo<T>::nb_read(T& data)
{
    bool res = Super::nb_read(data);
    if (log_fifo_log_ok_io <= log_fifo_logging_level)
    {
        if (res)
        {
            cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_readsuccess" << endl;
        }
        else
        {
            // the problem is lots of failures, not several from time to time!
            // set sensitivity etc. to avoid unnecessary invocations
            cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_readfailure" << endl;
        }
    }
    return res;
}

template<class T>
bool log_fifo<T>::nb_write(const T& data)
{
    bool res = Super::nb_write(data);
    if (log_fifo_log_notok_io <= log_fifo_logging_level)
    {
        if (!res)
        {
            cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_writefailure" << endl;
        }
        else
        {
            if (log_fifo_log_ok_io <= log_fifo_logging_level)
            {
                cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_writesuccess" << endl;
            }
        }
    }
    return res;
}

using inR = log_fifo<routing_block>;
typedef inR* inRP;
using inK = log_fifo<k_block>;
typedef inK* inKP;
using inSRQ = log_fifo<send_shadow_rq>;
typedef inSRQ* inSRQP;
using inSPN = log_fifo<recv_shadow_rq>;
typedef inSPN* inSPNP;
using inLB = log_fifo<lb_block>;
typedef inLB* inLBP;
using inMSGN = log_fifo<msgn_block>;
typedef inMSGN* inMSGNP;
using inM = log_fifo<m_block>;
typedef inM* inMP;
