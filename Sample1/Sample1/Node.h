#pragma once

extern int log_fifo_logging_level;
#define log_fifo_log_destructor 9
#define log_fifo_log_ok_io 10
#define log_fifo_log_notok_io 8

enum NodeType
{
	W = 0,
	S = 1,
	A = 2,
	Unknown = 999
};
char NTTS(NodeType nt);

struct k_block
{
	NodeType sender;
	int info1;
    int info2;
	int p_kblock;
	friend ostream& operator << (ostream& os, const k_block& k);
};
k_block create_k_block(int pkbl = 0, int inf1 = 0, int inf2 = 0, NodeType nt = NodeType::Unknown);
ostream& operator << (ostream& os, const k_block& k);

struct m_block
{
	NodeType sender;
	int info1;
    int info2;
    friend ostream& operator << (ostream& os, const m_block& k);
};
m_block create_m_block(int inf1 = 0, int inf2 = 0, NodeType nt = NodeType::Unknown);

struct send_shadow_rq
{
	NodeType sender;
	int info1;
    int info2;
    int info3;
	int leader;
    friend ostream& operator << (ostream& os, const send_shadow_rq& k);
};
send_shadow_rq create_send_shadow_rq(int leader = 0, int inf1 = 0, int inf2 = 0, int inf3 = 0, NodeType nt = NodeType::Unknown);

struct recv_shadow_rq
{
	NodeType sender;
	int info1;
    int info2;
    friend ostream& operator << (ostream& os, const recv_shadow_rq& k);
};
recv_shadow_rq create_recv_shadow_rq(int inf1 = 0, int inf2 = 0, NodeType nt = NodeType::Unknown);


template <class T>
struct log_fifo : sc_fifo<T>
{
    typedef sc_fifo<T> Super;

	log_fifo(sc_module_name name, int size_, int num = 0) : sc_fifo(name, size_)
	{

	}

    bool nb_read(T&) override;
    bool nb_write(const T&) override;

	~log_fifo()
	{
        if (log_fifo_log_destructor <= log_fifo_logging_level)
            cout << "Destructor,log_fifo," << name() << "." << endl;
        //cout << "Destructing log_fifo " << name() << "." << endl;
        //cout << "Destructed log_fifo '" << name() << "'." << endl;
    }
};

template<class T>
bool log_fifo<T>::nb_read(T& data)
{
    bool res = Super::nb_read(data);
    if (log_fifo_log_notok_io <= log_fifo_logging_level)
    {
        if (res)
        {
            cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_readsuccess" << endl;
            //cout << sc_time_stamp() << ": log_fifo " << name() << " nb_read success." << endl;
        }
        else
        {
            if (log_fifo_log_ok_io <= log_fifo_logging_level)
            {
                // делать sensitivity и пр., чтобы лишний раз не вызывались
                cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_readfailure" << endl;
                //cout << sc_time_stamp() << ": log_fifo " << name() << " nb_read failure." << endl;
            }
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
            //cout << sc_time_stamp() << ": log_fifo " << name() << " nb_write failure." << endl;
        }
        else
        {
            if (log_fifo_log_ok_io <= log_fifo_logging_level)
            {
                // делать sensitivity и пр., чтобы лишний раз не вызывались
                cout << "LogFifo," << sc_time_stamp() << "," << name() << "nb_writesuccess" << endl;
                //cout << sc_time_stamp() << ": log_fifo " << name() << " nb_write success." << endl;
            }
        }
    }
    return res;
}


SC_MODULE(example_fifo) {
	void example_fifo::m_drain_packets(void);
	void example_fifo::t_source1(void);
	void example_fifo::t_source2(void);
	// Constructor
	SC_CTOR(example_fifo) : packet_fifo("Fifo", 5) {
		SC_METHOD(m_drain_packets);
		SC_THREAD(t_source1);
		SC_THREAD(t_source2);
		// Size the packet_fifo to 5 ints.
		//log_fifo<k_block> packet_fifo("Fifo", 5);
	}

	// Declare the FIFO
	log_fifo<k_block> packet_fifo;
};

