#ifndef __TRANSPORT_PLUS_H__ 
#define __TRANSPORT_PLUS_H__

#include "tpl_inc.h"
#include "low_latency_chord.h"
#include "log_defs.h"



namespace P2P_MODEL
{

    class transport_plus : public sc_module,
                                 public MsgLog
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

        void config_req();
        void mess_req();
        void send_mess();
        void receive_mess();
        void set_network_address(const network_address& addr);
        network_address& get_network_address();

    public:
    //private:
        node_address& get_node_address();
    };

}
#endif