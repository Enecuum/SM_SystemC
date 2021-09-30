#ifndef __TRANSPORT_PLUS_H__ 
#define __TRANSPORT_PLUS_H__

#include "inc.h"
#include "low_latency_chord.h"
#include "log.h"



namespace P2P_MODEL
{

    class transport_plus : public sc_module,
                                 public log
    {
    private:

        sc_event m_eventConfigReq;
        sc_event m_eventMessReq;
        sc_event m_eventReceiveMess;
        sc_event m_eventSendMess;

        low_latency_chord m_llchord;

    public:

        SC_HAS_PROCESS(transport_plus);

        transport_plus(sc_module_name _name);
        ~transport_plus();

        void configReq();
        void messReq();
        void sendMess();
        void receiveMess();
        void setNetworkAddress(const network_address& addr);
        network_address& getNetworkAddress();

    public:
    //private:
        node_address& getNodeAddress();
    };

}
#endif