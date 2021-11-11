#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "inc.h"
#include "log.h"
#include "trp/message_buffer.h"
#include "trp/trp_network_if.h"
#include "net/network_trp_if.h"

namespace P2P_MODEL
{
    struct message_wake_up_info {
        int     bufIndex;
        sc_time time;
    };



    class network : public sc_module,
                    public log,
                    public network_trp_if
    {
    private:
        vector< sc_event* > m_eventSend;
        sc_event m_eventCheckReceive;

        vector< list <chord_byte_message> > m_buffMess;        
        map<uint160, uint> m_portIndexByNodeID;

        vector< vector<sc_time> > m_latencyTable;
       
       
        vector<message_wake_up_info> m_wakeUpInfo;
        vector<uint> m_randDesperseMillisec;

        bool m_hasNewMess;

        

        const int CAN_USE = -1;
        
    public:
        vector< sc_port<trp_network_if>* > trp_ports;

    public:

        SC_HAS_PROCESS(network);

        network(sc_module_name name, const uint nodes = 2);
        ~network();

        void push_into_network(const chord_byte_message& mess);

        void setNodeAddressList(const vector<network_address>& addrs);
        void setRandomLatencyTable(const uint millisecFrom, const uint millisecTo, const uint millisecDesperse);
        string latencyTableToStr();
                

    private:        
        void send();
        void deleteEventsPorts();
        void createNewEventPort();        
        void setNodeAmount(const uint nodess);
        void checkReceive();
        
        //void pushLatency(const uint160 nodeID, const sc_time latency);
        uint specifyNewMessType(const uint type);
    };
}
#endif