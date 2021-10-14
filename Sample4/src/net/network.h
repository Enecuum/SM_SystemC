#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "inc.h"
#include "log.h"
#include "trp/req_buffer.h"
#include "trp/trp_network_if.h"
#include "net/network_trp_if.h"

namespace P2P_MODEL
{


    class network : public sc_module,
                    public log,
                    public network_trp_if
    {
    private:
        vector< sc_event* > m_eventSend;        
        vector< vector<message_info> > m_messages;        
        map<uint160, uint> m_portIndexByNodeID;
        map<uint, sc_time> m_latencyByPortIndex;   
        vector<bool> m_hasMessageInBuffer;
        
    public:
        vector< sc_port<trp_network_if>* > trp_ports;

    public:

        SC_HAS_PROCESS(network);

        network(sc_module_name name, const uint nodes = 2);
        ~network();

        void push_into_network(const message_info& mess);

        void pushLatency(const uint160 nodeID, const sc_time latency);

        //void setLatencyTable();
                

    private:        
        void send();
        void deleteEventsPorts();
        void createNewEventPort();        
        void setNodeAmount(const uint nodess);
    };
}
#endif